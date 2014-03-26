<?php

/**
 * Views argument handler for dates.
 *
 * This code is full of junk because calendar and date modules are stupid.
 */
class Udate_ViewsHandlerArgument extends views_handler_argument_date
{
    /**
     * @see date_views_handler_is_date()
     * @see calendar_plugin_style::render()
     */
    public $min_date;

    /**
     * @see calendar_plugin_style::render()
     */
    public $max_date;

    /**
     * @see calendar_plugin_style::render()
     */
    public $date_range;

    /**
     * @see calendar_plugin_style::render()
     */
    public $limit;

    /**
     * (non-PHPdoc)
     *
     * Mostly copy/pasted code from views_handler_argument_formula.
     *
     * @see views_handler_argument_formula::query()
     */
    public function query($group_by = false)
    {
        $this->ensure_my_table();

        $date = null;

        if (is_numeric($this->argument) && (@$date = new DateTime('@' . $this->argument))) {
          // Ok got it.
            time();
        } else if (10 === strlen($this->argument) && (@$date = DateTime::createFromFormat('Y-m-d', $this->argument))) {
            // Ok got it.
            time();
        } else {
            // Nope.
        }

        $bounds = Udate_ViewsDateHelper::getBounds($date, $this->options['granularity']);

        if (null !== $date && $bounds) {

            $table = $this->table_alias;
            $field = $this->real_field;

            // @see calendar_plugin_style::render()
            if (class_exists('DateObject')) {
                $bounds[0] = new DateObject($bounds[0]->format(DateTime::ISO8601));
                $bounds[1] = new DateObject($bounds[1]->format(DateTime::ISO8601));
            }
            $this->date_range = $bounds;
            $this->min_date   = $bounds[0];
            $this->max_date   = $bounds[1];
            $this->limit      = array($date->format('Y') - 3, $date->format('Y') + 3);
            $this->query->add_field($table, $field, 'calendar_start_date');
            $this->query->add_field($table, $field);

            switch ($this->options['granularity']) {

              case UDATE_GRANULARITY_SECOND:
                  $this->query->add_where_expression(
                      0,
                      $table . "." . $field
                          . " = '" . $date->format(UDATE_PHP_DATETIME) . "'"
                  );
                  break;

              default:
                  $this->query->add_where_expression(
                      0,
                      $table . "." . $field
                          . " BETWEEN '" . $bounds[0]->format(UDATE_PHP_DATETIME)
                          . "' AND '" . $bounds[1]->format(UDATE_PHP_DATETIME) . "'"
                  );
                  break;
            }
        }
    }

    public function option_definition()
    {
        $options = parent::option_definition();
        $options['granularity'] = array('default' => UDATE_GRANULARITY_SECOND);

        return $options;
    }

    public function options_form(&$form, &$form_state)
    {
        parent::options_form($form, $form_state);

        $form['granularity'] = array(
            '#title'         => t('Granularity'),
            '#type'          => 'radios',
            '#default_value' => $this->options['granularity'],
            '#options'       => array(
                UDATE_GRANULARITY_SECOND => t("Second"),
                UDATE_GRANULARITY_MINUTE => t("Minute"),
                UDATE_GRANULARITY_HOUR   => t("Hour"),
                UDATE_GRANULARITY_DAY    => t("Day"),
                UDATE_GRANULARITY_MONTH  => t("Month"),
                UDATE_GRANULARITY_YEAR   => t("Year"),
            ),
        );
    }
}
