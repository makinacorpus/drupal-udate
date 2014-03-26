<?php

/**
 * @file
 * Views integration.
 *
 * You probably don't want to use views anyway.
 */

/**
 * Implements hook_field_views_data_alter().
 */
function udate_field_views_data_alter(&$result, $field, $module) {
  switch ($field['type']) {

    case 'udate':
      $column = _field_sql_storage_columnname($field['field_name'], 'date');
      $result[$column]['argument']['handler'] = 'Udate_ViewsHandlerArgument';
      $result[$column]['filter']['handler'] = 'Udate_ViewsHandlerFilter';
      break;

    case 'udate_range':
      foreach ($result as $table => $data) {
        foreach (array('date_start', 'date_end') as $column) {
          $column = _field_sql_storage_columnname($field['field_name'], 'date_start');
          $result[$table][$column]['argument']['handler'] = 'Udate_ViewsHandlerArgument';
          $result[$table][$column]['filter']['handler'] = 'Udate_ViewsHandlerFilter';
        }
      }
      break;
  }
}

/**
 * Implements of hook_views_plugins_alter().
 */
function udate_views_plugins_alter(&$data) {
  if (isset($data['style']['calendar_style'])) {
    $data['style']['calendar_style']['handler'] = 'Udate_CalendarStylePlugin';
  }
  if (isset($data['row']['calendar_node'])) {
    $data['row']['calendar_node']['handler'] = 'Udate_CalendarEntityRowPlugin';
  }
  if (isset($data['row']['calendar_entity'])) {
    $data['row']['calendar_entity']['handler'] = 'Udate_CalendarEntityRowPlugin';
  }
}
