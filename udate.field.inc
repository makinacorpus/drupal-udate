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
        'dateFormat'  => UDATE_DATE_FORMAT_DEFAULT,
        'granularity' => null,
        'timeTitle'   => null,
      ),
    ),
    'udate_range' => array(
      'label' => t("Date picker"),
      'field types' => array('udate_range'),
      'settings' => array(
        'dateFormat'  => UDATE_DATE_FORMAT_DEFAULT,
        'granularity' => null,
        'timeTitle'   => null,
      ),
    ),
    // Next two are useful if you wish to replace the 'date' module field
    // widget with this one, which will probably be, in the end, much more
    // stable, efficient and surprise-less.
    'udate_legacy_date' => array(
      'label' => t("Date picker"),
      'field types' => array('datestamp', 'datetime'),
      'settings' => array(
        'dateFormat'  => UDATE_DATE_FORMAT_DEFAULT,
        'granularity' => null,
        'timeTitle'   => null,
      ),
    ),
  );
}

/**
 * Implements hook_field_widget_settings_form().
 */
function udate_field_widget_settings_form($field, $instance) {

  $element = array();

  switch ($instance['widget']['type']) {

    case 'udate':
    case 'udate_range':
    case 'udate_legacy_date':

      $element['dateFormat'] = array(
        '#title'          => t("Date format in widget"),
        '#type'           => 'textfield',
        '#default_value'  => $instance['widget']['settings']['dateFormat'],
        '#description'    => t("Widget works without JS even thought it's a jQuery datepicker, this determines the date format being used in the textfield. Please be aware that what is being asked here is a jQuery datepicker date format, not a PHP date format."),
      );

      $element['timeTitle'] = array(
        '#title'          => t("Time widget title"),
        '#type'           => 'textfield',
        '#default_value'  => $instance['widget']['settings']['timeTitle'],
      );

      // @todo This not logicial we should limit by the only possible values
      // but this will imply we can't transparently support the 'date' module
      // fields.
      $min = UDATE_GRANULARITY_SECOND; // empty($field['settings']['collectTime']) ? UDATE_GRANULARITY_DAY : UDATE_GRANULARITY_SECOND;
      $max = UDATE_GRANULARITY_YEAR;

      $element['granularity'] = array(
        '#title'          => t("Granularity"),
        '#type'           => 'select',
        '#options'        => udate_granularity_list($min, $max),
        '#empty_option'   => t("Field default setting"),
        '#default_value'  => $instance['widget']['settings']['granularity'],
      );
      break;
  }

  return $element;
}

/**
 * Value callback for 'date' module widgets.
 */
function udate_legacy_field_widget_form_validate(&$element, &$form_state, $form) {

  $value = array('value' => null, 'value2' => null, 'timezone' => $element['#timezone']);

  switch ($element['#fieldType']) {

    case 'datetime':
      if (isset($element['value']) && $element['value']['#value'] instanceof DateTime) {
        $value['value'] = $element['value']['#value']->format(UDATE_PHP_DATETIME);
      }
      if (isset($element['value2']) && $element['value2']['#value'] instanceof DateTime) {
        $value['value2'] = $element['value2']['#value']->format(UDATE_PHP_DATETIME);
      }
      break;

    case 'datestamp':
      if (isset($element['value']) && $element['value']['#value'] instanceof DateTime) {
        $value['value'] = $element['value']['#value']->getTimestamp();
      }
      if (isset($element['value2']) && $element['value2']['#value'] instanceof DateTime) {
        $value['value2'] = $element['value2']['#value']->getTimestamp();
      }
      break;
  }

  form_set_value($element, $element['#value'] = $value, $form_state);
}

/**
 * Get default timezone for field item.
 */
function udate_legacy_field_item_timezone_get($item, $field) {
  // Date module is so stupid.
  if (empty($item['timezone']) || empty($field['settings']['tz_handling']) || 'none' === $field['settings']['tz_handling']) {
    // More or less the same that the 'date' module would do, without being
    // hardwired to the 'date' module functions.
    global $user;
    if (variable_get('configurable_timezones', 1) && !empty($user->timezone)) {
      $timezone = $user->timezone;
    } else {
      $timezone = variable_get('date_default_timezone', 'UTC');
    }
  } else {
    $timezone = $item['timezone'];
  }
  return $timezone;
}

/**
 * Implements hook_field_widget_form() for 'date' module widgets.
 */
function udate_legacy_field_widget_form(&$form, &$form_state, $field, $instance, $langcode, $items, $delta, $element) {

  if (empty($field['settings']['todate'])) {
    $keys = array('value' => null);
  } else {
    $keys = array('value' => t("Start date"), 'value2' => t("End date"));
  }

  if (!empty($keys)) {

    $granularity  = $field['settings']['granularity'];
    $elementType  = empty($granularity['hour']) ? 'udate' : 'udatetime';
    $dateFormat   = $instance['widget']['settings']['dateFormat'];
    $timeTitle    = $instance['widget']['settings']['timeTitle'];

    // Restore granularity from the date module stuff.
    if (empty($granularity['month'])) {
      $granularity = UDATE_GRANULARITY_YEAR;
    } if (empty($granularity['day'])) {
      $granularity = UDATE_GRANULARITY_MONTH;
    } if (empty($granularity['hour'])) {
      $granularity = UDATE_GRANULARITY_DAY;
    } if (empty($granularity['minute'])) {
      $granularity = UDATE_GRANULARITY_HOUR;
    } else if (empty($granularity['second'])) {
      $granularity = UDATE_GRANULARITY_MINUTE;
    } else {
      $granularity = UDATE_GRANULARITY_SECOND;
    }

    if (isset($items[$delta])) {
      $timezone = udate_legacy_field_item_timezone_get($items[$delta], $field);
    } else {
      $timezone = udate_legacy_field_item_timezone_get(array(), $field);
    }

    foreach ($keys as $key => $title) {

      $element[$key] = array(
        '#title'        => $title,
        '#type'         => $elementType,
        '#granularity'  => $granularity,
        '#dateFormat'   => $dateFormat,
        '#timeTitle'    => $timeTitle,
      );

      if (!empty($items[$delta][$key])) {
        $value = null;

        switch ($field['type']) {

          case 'datetime':
            try {
              $value = DateTime::createFromFormat(UDATE_PHP_DATETIME, $items[$delta][$key]);
            } catch (Exception $e) {
              trigger_error($e->getMessage(), E_USER_WARNING);
              $value = null;
            }
            break;

          case 'datestamp':
            try {
              $value = new DateTime('@' . $items[$delta][$key]);
            } catch (Exception $e) {
              trigger_error($e->getMessage(), E_USER_WARNING);
              $value = null;
            }
            break;
        }

        if ($value instanceof DateTime) {
          // Ok, now we have a date module problem, don't let him change our
          // values behind our backs and translate it while we still can.
          // Because this stupid module actually did translate our item.
          $from = 'UTC';
          if (!empty($items[$key]['timezone_db'])) {
            $from = $items[$key]['timezone_db'];
          }
          if ($from !== $timezone) {
            $value->setTimezone(new DateTimeZone($timezone));
          }
        }

        $element[$key]['#default_value'] = $value;
      }
    }

    $element['granularity'] = array(
      '#type'  => 'value',
      '#value' => $granularity,
    );
  }

  $element['#fieldType'] = $field['type'];
  $element['#timezone'] = $timezone;
  $element['#element_validate'][] = 'udate_legacy_field_widget_form_validate';

  return $element;
}

/**
 * Implements hook_field_widget_form().
 */
function udate_field_widget_form(&$form, &$form_state, $field, $instance, $langcode, $items, $delta, $element) {

  if ('udate_legacy_date' === $instance['widget']['type']) {
    return udate_legacy_field_widget_form($form, $form_state, $field, $instance, $langcode, $items, $delta, $element);
  }

  switch ($instance['widget']['type']) {

    case 'udate':
      $keys = array('date' => null);
      break;

    case 'udate_range':
      $keys = array('date_start' => t("Start date"), 'date_end' => t("End date"));
      break;
  }

  if (!empty($keys)) {

    $elementType  = $instance['settings']['collectTime'] ? 'udatetime' : 'udate';
    $granularity  = $instance['settings']['granularity'];
    $dateFormat   = $instance['widget']['settings']['dateFormat'];
    $timeTitle    = $instance['widget']['settings']['timeTitle'];

    foreach ($keys as $key => $title) {

      $element[$key] = array(
        '#title'        => $title,
        '#type'         => $elementType,
        '#granularity'  => $granularity,
        '#dateFormat'   => $dateFormat,
        '#timeTitle'    => $timeTitle,
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
