<?php

/**
 * This class is full of shit, because calendar and date modules are both
 * huge shitbags. Shit happens, deal with it.
 */
class Udate_CalendarEntityRowPlugin extends calendar_plugin_row
{
    protected $isUdate = false;

    protected $udateField;

    /**
     * This actually was a good idea
     */
    protected function preloadEntities($values)
    {
        $ids = array();

        foreach ($values as $row) {
            $id = $row->{$this->field_alias};

            if ($this->view->base_table == 'node_revision') {
                $this->entities[$id] = node_load(NULL, $id);
            } else {
                $ids[$id] = $id;
            }
        }

        $base_tables = date_views_base_tables();
        $this->entity_type = $base_tables[$this->view->base_table];

        if (!empty($ids)) {
            $this->entities = entity_load($this->entity_type, $ids);
        }
    }

    public function pre_render($values)
    {
        foreach ($this->view->argument as $handler) {
            if (date_views_handler_is_date($handler, 'argument')) {
                if ($handler instanceof Udate_ViewsHandlerArgument) {

                    $this->isUdate = true;
                    $this->udateField = str_replace(array('_granularity', '_date_start', '_date_end'), '', $handler->real_field);
                    $this->language = LANGUAGE_NONE;
                    $this->date_fields = array();
                    $this->date_argument = $handler;
                    $this->preloadEntities($values);

                    return;
                }
            }
        }

        parent::pre_render($values);
    }

    public function render($row)
    {
        if (!$this->isUdate) {
            return parent::render($row);
        }

        $rows = array();
        $id = $row->{$this->field_alias};

        if (!is_numeric($id)) {
            return $rows;
        }

        // Load the specified node:
        // We have to clone this or nodes on other views on this page,
        // like an Upcoming block on the same page as a calendar view,
        // will end up acquiring the values we set here.
        $entity = clone($this->entities[$id]);

        if (empty($entity)) {
            return $rows;
        }

        $info = entity_get_info($this->entity_type);
        $this->id_field = $info['entity keys']['id'];
        $this->id = $entity->{$this->id_field};
        $this->type = !empty($info['entity keys']['bundle']) ? $info['entity keys']['bundle'] : $this->entity_type;
        $this->title = entity_label($this->entity_type, $entity);
        $uri = entity_uri($this->entity_type, $entity);
        $uri['options']['absolute'] = TRUE;
        $this->url = url($uri['path'], $uri['options']);
        $field_name = $this->udateField;
        $entity->date_id = array();
        $granularity = 'second';
        $this->date_argument->view->date_info->display_timezone_name = date_default_timezone_get();

        // Set the date_id for the node, used to identify which field
        // value to display for fields that have multiple values. The
        // theme expects it to be an array.
        $date_id = 'date_id_' . $field_name;
        $date_delta = 'date_delta_' . $field_name;
        $delta = 0;

        if (isset($row->$date_id)) {
            $entity->date_id = array('calendar.' . $row->$date_id . '.' . $field_name. '.' . $delta);
        } else {
            $entity->date_id = array('calendar.' . $id . '.' . $field_name . '.' . $delta);
        }

        if (!($items = field_get_items($this->entity_type, $entity, $field_name)) || !isset($items[$delta])) {
            return;
        }
        $item = $items[$delta];
        $fieldinfo = field_info_field($field_name);
        switch ($fieldinfo['type']) {

            case 'udate':
                $item_start_date = $item['date'];
                break;

            case 'udate_range':
                $item_start_date = $item['date_start'];
                $item_end_date   = $item['date_end'];
                break;

            default:
                return;
        }

        if (empty($item_start_date)) {
            return;
        }
        if (empty($item_end_date)) {
            $item_end_date = $item_start_date;
        }
        if (class_exists('DateObject')) {
            $item_start_date = new DateObject($item_start_date->format(DateTime::ISO8601));
            $item_end_date   = new DateObject($item_end_date->format(DateTime::ISO8601));
        }

        $event = new stdClass();
        $event->id = $this->id;
        $event->title = $this->title;
        $event->type = $this->type;
        $event->date_start = $item_start_date;
        $event->date_end = $item_end_date;
        $event->db_tz = date_default_timezone_get();
        $event->to_zone = date_default_timezone_get();
        $event->granularity = $granularity;
        $event->increment = 1;
        $event->field = $item;
        $event->url = $this->url;
        $event->row = $row;
        $event->entity = $entity;
        $event->stripe = array();
        $event->stripe_label = array();

        // All calendar row plugins should provide a date_id that the
        // theme can use.
        $event->date_id = $entity->date_id[0];

        // We are working with an array of partially rendered items
        // as we process the calendar, so we can group and organize them.
        // At the end of our processing we'll need to swap in the fully formatted
        // display of the row. We save it here and switch it in
        // template_preprocess_calendar_item().
        $event->rendered = theme(
            $this->theme_functions(),
            array(
                'view'        => $this->view,
                'options'     => $this->options,
                'row'         => $row,
                'field_alias' => isset($this->field_alias) ? $this->field_alias : '',
            )
        );

        $entities = $this->explode_values($event);
        foreach ($entities as $entity) {
            switch ($this->options['colors']['legend']) {

                case 'type':
                    $this->calendar_node_type_stripe($entity);
                    break;

                case 'taxonomy':
                    $this->calendar_taxonomy_stripe($entity);
                    break;

                case 'group':
                    $this->calendar_group_stripe($entity);
                    break;
            }
            $rows[] = $entity;
        }

        return $rows;
    }
}
