<?php

/**
 * @file
 * Field handling related functions.
 *
 * Warning: this may contain some serious hacks.
 */

/**
 * Implements hook_field_info().
 */
function udate_field_info() {
  return array(
    'udate' => array(
      'label' => t("Date (µDate)"),
      'description' => t("Stores a date or a datetime"),
      'settings' => array(),
      'instance_settings' => array(
        'collectTime' => false,
        'granularity' => UDATE_GRANULARITY_SECOND,
      ),
      'default_widget' => 'udate',
      'default_formatter' => 'udate',
    ),
    'udate_range' => array(
      'label' => t("Date range (µDate)"),
      'description' => t("Stores a date or a datetime range"),
      'settings' => array(),
      'instance_settings' => array(
        'collectTime' => false,
        'granularity' => UDATE_GRANULARITY_SECOND,
      ),
      'default_widget' => 'udate_range',
      'default_formatter' => 'udate_range',
    ),
  );
}

/**
 * Implements hook_field_widget_info().
 */
function udate_field_formatter_info() {
  return array(
    'udate' => array(
      'label' => t("Drupal date format"),
      'field types' => array('udate'),
      'settings' => array(
        'drupalDateFormat' => 'normal',
      ),
    ),
    'udate_range' => array(
      'label' => t("Drupal date format"),
      'field types' => array('udate_range'),
      'settings' => array(
        'drupalDateFormat' => 'normal',
      ),
    ),
  );
}

/**
 * Implements hook_field_formatter_settings_form().
 */
function udate_field_formatter_settings_form($field, $instance, $view_mode, $form, &$form_state) {

  $display  = $instance['display'][$view_mode];
  $settings = $display['settings'];
  $element  = array();

  // @todo

  return $element;
}

/**
 * Implements hook_field_formatter_settings_summary().
 */
function udate_field_formatter_settings_summary($field, $instance, $view_mode) {

  $display  = $instance['display'][$view_mode];
  $settings = $display['settings'];
  $summary  = '';

  // @todo

  return $summary;
}

/**
 * Implements hook_field_widget_info().
 */
function udate_field_widget_info() {
  return array(
    'udate' => array(
      'label' => t("Date picker"),
      'field types' => array('udate'),
      'settings' => array(
        'dateFormat' => UDATE_DATE_FORMAT_DEFAULT,
      ),
    ),
    'udate_range' => array(
      'label' => t("Date picker"),
      'field types' => array('udate_range'),
      'settings' => array(
        'dateFormat' => UDATE_DATE_FORMAT_DEFAULT,
      ),
    ),
  );
}

/**
 * Implements hook_field_widget_settings_form().
 */
function udate_field_widget_settings_form($field, $instance) {

  $element = array();

  switch ($instance['widget']) {

    case 'udate':
    case 'udate_range':
      $element['dateFormat'] = array(
        '#title'         => t("Date format in widget"),
        '#type'          => 'textfield',
        '#default_value' => $widget['settings']['dateFormat'],
        '#description'   => t("Widget works without JS even thought it's a jQuery datepicker, this determines the date format being used in the textfield. Please be aware that what is being asked here is a jQuery datepicker date format, not a PHP date format."),
      );
      break;
  }

  return $element;
}

/**
 * Implements hook_field_widget_form().
 */
function udate_field_widget_form(&$form, &$form_state, $field, $instance, $langcode, $items, $delta, $element) {

  switch ($instance['widget']['type']) {

    case 'udate':
      $keys = array('date' => null);
      break;

    case 'udate_range':
      $keys = array('date_start' => t("Start date"), 'date_end' => t("End date"));
      break;
  }

  if (!empty($keys)) {

    $elementType   = $instance['settings']['collectTime'] ? 'udatetime' : 'udate';
    $granularity   = $instance['settings']['granularity'];
    $dateFormat    = $instance['widget']['settings']['dateFormat'];
    $phpDateFormat = udate_format_convert($dateFormat);

    foreach ($keys as $key => $title) {
      $element[$key] = array(
        '#title'       => $title,
        '#type'        => $elementType,
        '#granularity' => $granularity,
        '#dateFormat'  => $dateFormat,
      );
      if (!empty($items[$delta][$key])) {
        // Invalid dates will become null so that's not a problem for us.
        $element[$key]['#default_value'] = $items[$delta][$key];
      }
    }

    $element['granularity'] = array(
      '#type'  => 'value',
      '#value' => $granularity,
    );
  }

  return $element;
}

/**
 * Implements hook_field_attach_presave().
 *
 * We use this hook instead of hook_field_presave() because it gets executed
 * just before the real save, and allows us to avoid potential conflicts with
 * other modules.
 *
 * @see udate_field_attach_insert()
 * @see udate_field_attach_update()
 */
function udate_field_attach_presave($entity_type, $entity) {
  list(,, $bundle) = entity_extract_ids($entity_type, $entity);
  foreach (field_info_instances($entity_type, $bundle) as $instance) {

    $field_name = $instance['field_name'];
    if (!$field = field_info_field($field_name)) {
      continue;
    }

    $keys = null;
    switch ($field['type']) {

      case 'udate':
        $keys = array('date');
        break;

      case 'udate_range':
        $keys = array('date_start', 'date_end');
        break;
    }

    // Field API magic: foreach-ception.
    if (!empty($keys) && !empty($entity->{$field_name})) {
      foreach ($entity->{$field_name} as $language => &$items) {
        foreach ($items as $delta => &$item) {
          foreach ($keys as $key) {
            if (!empty($item[$key])) {
              if ($item[$key] instanceof DateTime) {
                $item[$key] = $item[$key]->format(UDATE_PHP_DATETIME);
                // PDOException : SQLSTATE[23000]: Integrity constraint violation: 1048
                // Column 'planning_granularity' cannot be null: INSERT INTO {field_data_planning}
                // @todo Check this has no other implications.
                if (empty($item['granularity'])) {
                  $item['granularity'] = 0;
                }
              }
            }
          }
        }
      }
    }
  }
}

/**
 * Implements hook_field_load().
 */
function udate_field_load($entity_type, $entities, $field, $instances, $langcode, &$items, $age) {

  if (empty($entities)) {
    return;
  }

  switch ($field['type']) {

    case 'udate':
      $keys = array('date');
      break;

    case 'udate_range':
      $keys = array('date_start', 'date_end');
      break;
  }

  if (!empty($keys)) {
    foreach ($items as $id => &$eitems) {
      foreach ($eitems as $delta => &$item) {
        foreach ($keys as $key) {
          if (!empty($item[$key])) {
            $item[$key] = DateTime::createFromFormat(UDATE_PHP_DATETIME, $item[$key]);
          }
        }
      }
    }
  }
}

/**
 * Fixes fields internal values to be DateTime objects after saving fields.
 *
 * The field API does not allow to proceed to field data transformation between
 * the storage layer and the data we have in PHP userland code, so we need to
 * attach ourselves on hook_field_load() to convert DATETIME strings to DateTime
 * objects, then attach ourselves after save on those exact same fields in order
 * to convert back the value from DATETIME string to DateTime object.
 *
 * Drupal is so wrong, in many ways, and this is one, really. They never get the
 * chance to known anything about computer science and relational databases.
 */
function udate_field_attach_fix($entity_type, $entity) {
  list(,, $bundle) = entity_extract_ids($entity_type, $entity);
  foreach (field_info_instances($entity_type, $bundle) as $instance) {

    $field_name = $instance['field_name'];
    if (!$field = field_info_field($field_name)) {
      continue;
    }

    $keys = null;
    switch ($field['type']) {

      case 'udate':
        $keys = array('date');
        break;

      case 'udate_range':
        $keys = array('date_start', 'date_end');
        break;
    }

    // Field API magic: foreach-ception.
    if (!empty($keys) && !empty($entity->{$field_name})) {
      foreach ($entity->{$field_name} as $language => &$items) {
        foreach ($items as $delta => &$item) {
          foreach ($keys as $key) {
            if (!empty($item[$key])) {
              if (!$item[$key] instanceof DateTime) {
                $item[$key] = DateTime::createFromFormat(UDATE_PHP_DATETIME, $item[$key]);
              }
            }
          }
        }
      }
    }
  }
}

/**
 * Implements hook_field_attach_insert().
 */
function udate_field_attach_insert($entity_type, $entity) {
  udate_field_attach_fix($entity_type, $entity);
}

/**
 * Implements hook_field_attach_update().
 */
function udate_field_attach_update($entity_type, $entity) {
  udate_field_attach_fix($entity_type, $entity);
}

/**
 * Implements hook_field_is_empty().
 */
function udate_field_is_empty($item, $field) {
  switch ($field['type']) {

    case 'udate':
      return empty($item['date']);

    case 'udate_range':
      // Only start date is mandatory. For managing a calendar you will need
      // all the events whether or not they are ponctual or a range to live
      // in the same field. Start date serves the purpose of managing single
      // dates using a date range field.
      return empty($item['date_start']);
  }

  return false;
}
