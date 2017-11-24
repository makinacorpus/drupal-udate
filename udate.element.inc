<?php

/**
 * @file
 * Element info and widget related functions.
 *
 * Go ahead please and do better you bitch!
 */

/**
 * Expands the udate date element.
 */
function udate_element_date_process($element) {

  $element['#tree'] = true;
  unset($element['#input']);

  // Validate the input.
  if (isset($element['#value']) && $element['#value'] instanceof DateTime) {
    $value = $element['#value'];
  } else {
    $value = $element['#value'] = null;
  }

  // Validate the min and max dates.
  if (isset($element['#minDate']) && $element['#minDate'] instanceof DateTime) {
    $minDate = $element['#minDate'];
  } else {
    unset($element['#minDate']);
    $minDate = null;
  }
  if (isset($element['#maxDate']) && $element['#maxDate'] instanceof DateTime) {
    $maxDate = $element['#maxDate'];
  } else {
    unset($element['#maxDate']);
    $maxDate = null;
  }

  $phpFormat = udate_format_convert($element['#dateFormat']);

  $settings = [];
  if (!empty($element['#settings'])) {
    $settings = $element['#settings'];
  }

  $settings += array(
    'dayNamesMin'   => array(t("Su"), t("Mo"), t("Tu"), t("We"), t("Th"), t("Fr"), t("Sa")),
    'monthNames'    => array(t("January"), t("February"), t("March"), t("April"), t("May"), t("June"), t("July"), t("August"), t("September"), t("October"), t("November"), t("December")),
    'firstDay'      => 1,
    'minDate'       => $minDate ? $minDate->format($phpFormat) : null,
    'maxDate'       => $maxDate ? $maxDate->format($phpFormat) : null,
    'dateFormat'    => $element['#dateFormat'],
    'phpDateFormat' => $phpFormat,
    'defaultDate'   => $value ? $value->format($phpFormat) : null,
    'allowedRanges' => [],
  );

  if (!empty($element['#ranges'])) {
    // Validate range input
    foreach ($element['#ranges'] as $index => $range) {
      if (!is_array($range) && 2 !== count($range) || !preg_match('/^\d+-\d+$/', $range[0]) || !preg_match('/^\d+-\d+$/', $range[1])) {
        trigger_error("malformed range, dropping", E_USER_WARNING);
        unset($element['#ranges'][$index]);
        continue;
      }
      if ($range[1] < $range[0]) {
        trigger_error("malformed range: first date must be prior to second date, dropping", E_USER_WARNING);
        unset($element['#ranges'][$index]);
        continue;
      }
      $settings['allowedRanges'][] = $range;
    }
  }

  // Compute a predictable unique identifier for the field (there might be
  // more than one on the same screen).
  if (!empty($element['#field_parents'])) {
    // This is a field widget.
    $id = drupal_clean_css_identifier(implode('-', $element['#field_parents']));
  } else if (false) {
    // This is a nested form element.
  } else {
    // We don't have enough information, this can break AJAX.
    $id = drupal_clean_css_identifier($element['#id']);
  }

  $now = new DateTime();

  // Spawn the real widget.
  $element['date'] = array(
    '#title'          => isset($element['#title']) ? $element['#title'] : null,
    '#id'             => $id,
    '#type'           => 'textfield',
    '#attributes'     => array(
      'placeholder'   => $element['#placeholder'] ? $element['#placeholder'] : $now->format($phpFormat),
      'title'         => t("Date format is @format", array('@format' => $element['#dateFormat'])),
    ),
    '#default_value'  => isset($value) ? $value->format($phpFormat) : null,
    '#size'           => 10,
    '#maxlength'      => 10,
    '#required'       => isset($element['#required']) ? $element['#required'] : false,
    '#description'    => isset($element['#description']) ? $element['#description'] : null,
  );

  // Propagate AJAX settings
  if (!empty($element['#ajax'])) {
    $element['date']['#ajax'] = $element['#ajax'];
    unset($element['#ajax']);
  }

  $element['#required'] = false; // Avoid stupid FAPI errors.

  $element['#settings'] = $settings;

  // Attach the jQuery date picker settings, that will be caught by our custom
  // JavaScript file and passed to the date picker init.
  $element['#attached']['js'][] = array(
    'type' => 'setting',
    'data' => array('udate' => array($id => array_filter($settings))),
  );
  // We already attached the library into some hook_init() but this code is
  // not a dependency of this, so let's assume it might not be set.
  if (!variable_get(UDATE_VAR_JS_ALL, true)) {
    $element['#attached']['library'][] = array('udate', 'udate');
  }

  return $element;
}

/**
 * Validates the udate date element.
 */
function udate_element_date_validate(&$element, &$form_state, $form) {

  /** @var \DateTimeInterface $value */
  $value = $element['date']['#value'];

  // Value can be null?
  if (empty($element['#required']) && empty($value)) {
    $element['#value'] = null;
    $element['date']['#value'] = null;
    form_set_value($element, null, $form_state);
    return;
  }

  // Ensure the date is correctly formatted.
  if (!$value = DateTime::createFromFormat($element['#settings']['phpDateFormat'] . ' H:i:s', $value . ' 00:00:00')) {
    form_set_error(implode('][', $element['#parents']), t("Date format is invalid"));
    return;
  }

  // Validate allowed ranges.
  if (isset($element['#ranges'])) {
    $current = $value->format('m-d');
    $atLeastOne = false;
    foreach ($element['#ranges'] as $range) {
      list($start, $stop) = $range;
      if ($start <= $current && $current <= $stop) {
        $atLeastOne = true;
        break;
      }
    }
    if (!$atLeastOne) {
      if (empty($element['#rangesMessage'])) {
        form_set_error(implode('][', $element['#parents']), t("Selected day is not within the allowed ranges"));
      } else {
        form_set_error(implode('][', $element['#parents']), $element['#rangesMessage']);
      }
    }
  }

  if (isset($element['#minDate']) && $value < $element['#minDate']) {
    form_set_error(implode('][', $element['#parents']), t("Date must be over %s", array('%s' => $element['#settings']['minDate'])));
    return;
  }
  if (isset($element['#maxDate']) && $element['#maxDate'] < $value) {
    form_set_error(implode('][', $element['#parents']), t("Date must be under %s", array('%s' => $element['#settings']['maxDate'])));
    return;
  }

  form_set_value($element, $element['#value'] = $value, $form_state);
}

/**
 * Expands the udate time element.
 */
function udate_element_time_process($element) {

  unset($element['#input']);

  /* @var $value \DateInterval */
  if (isset($element['#value']) && $element['#value'] instanceof DateInterval) {
    $value = $element['#value'];
  } else {
    $value = $element['#value'] = null;
  }

  $now = new DateTime();

  if (UDATE_GRANULARITY_MINUTE < $element['#granularity']) {
    $hint     = 'HH';
    $format   = 'H';
    $default  = $value ? $value->format(UDATE_TIME_FORMAT_HOUR) : null;
  } else if (UDATE_GRANULARITY_SECOND < $element['#granularity']) {
    $hint     = 'HH:MM';
    $format   = 'H:i';
    $default  = $value ? $value->format(UDATE_TIME_FORMAT_MINUTE) : null;
  } else {
    $hint     = 'HH:MM:SS';
    $format   = 'H:i:s';
    $default  = $value ? $value->format(UDATE_TIME_FORMAT_DEFAULT) : null;
  }

  $element['time'] = array(
    '#title'          => isset($element['#title']) ? $element['#title'] : null,
    '#title_display'  => isset($element['#title_display']) ? $element['#title_display'] : null,
    '#field_prefix'   => isset($element['#field_prefix']) ? $element['#field_prefix'] : null,
    '#field_suffix'   => isset($element['#field_suffix']) ? $element['#field_suffix'] : null,
    '#type'           => 'textfield',
    '#attributes'     => array(
      'placeholder'   => $element['#placeholder'] ? $element['#placeholder'] : $now->format($format),
      'title'         => t("Time format is @format", array('@format' => $hint))
    ),
    '#default_value'  => $default,
    '#size'           => strlen($hint),
    '#maxlength'      => strlen($hint),
    '#required'       => isset($element['#required']) ? $element['#required'] : false,
  );

  // Propagate AJAX settings
  if (!empty($element['#ajax'])) {
    $element['time']['#ajax'] = $element['#ajax'];
    unset($element['#ajax']);
  }

  // Special use case, leave the field empty when the hour is set to
  // just all zeros, it seems legit that the user probably didn't fill
  // any value here.
  if ($value && !$element['#required'] && '000' === $value->format('%h%i%s')) {
    $element['time']['#default_value'] = '';
  }

  return $element;
}

/**
 * Validates the udate time element.
 */
function udate_element_time_validate(&$element, &$form_state, $form) {

  $value = $element['time']['#value'];

  // Value can be null?
  if (!$element['#required'] && empty($value)) {
    $element['#value'] = null;
    $element['time']['#value'] = null;
    form_set_value($element, null, $form_state);
    return;
  }

  // Ensure the date is correctly formatted.
  $parts = array_filter(explode(':', $value));

  if (empty($parts)) {
    form_set_error(implode('][', $element['#parents']), t("Time format is invalid"));
    return;
  }

  $intervalSpec = 'PT';
  foreach (array('H', 'M', 'S') as $i => $spec) {
    if (empty($parts[$i])) {
      break;
    }
    $intervalSpec .= $parts[$i] . $spec;
  }

  try {
    $value = new DateInterval($intervalSpec);
  } catch (Exception $e) {
    form_set_error(implode('][', $element['#parents']), t("Time format is invalid"));
    return;
  }

  form_set_value($element, $element['#value'] = $value, $form_state);
}

/**
 * Expands the udate datetime element.
 */
function udate_element_datetime_process($element) {

  unset($element['#input']);

  if (isset($element['#value']) && $element['#value'] instanceof DateTime) {
    $value = $element['#value'];
  } else {
    $value = $element['#value'] = null;
  }

  $element['container'] = array(
    '#prefix' => '<div class="container-inline">',
    '#suffix' => '</div>',
  );

  $element['container']['date'] = array(
    '#title'          => isset($element['#title']) ? $element['#title'] : null,
    '#type'           => 'udate',
    '#required'       => isset($element['#required']) ? $element['#required'] : false,
    '#minDate'        => $element['#minDate'],
    '#maxDate'        => $element['#maxDate'],
    '#dateFormat'     => $element['#dateFormat'],
    '#ranges'         => $element['#ranges'],
    '#default_value'  => $value,
    '#placeholder'    => $element['#date_placeholder'],
  );

  $element['container']['time'] = array(
    '#type'           => 'utime',
    '#title'          => empty($element['#timeTitle']) ? null : $element['#timeTitle'],
    '#required'       => isset($element['#required']) ? $element['#required'] : false,
    '#granularity'    => $element['#granularity'],
    '#default_value'  => $value ? new DateInterval($value->format('\P\TH\Hi\Ms\S')) : null,
    '#placeholder'    => $element['#time_placeholder'],
  );

  return $element;
}

/**
 * Validates the udate datetime element.
 */
function udate_element_datetime_validate(&$element, &$form_state, $form) {

  $date = $element['container']['date']['#value'];
  $time = $element['container']['time']['#value'];

  // Value can be null?
  if (empty($element['#required']) && empty($date)) {
    form_set_value($element, $element['#value'] = null, $form_state);
    return;
  }

  // But time can be.
  if (!empty($time)) {
    $date->add($time);
  }

  form_set_value($element, $element['#value'] = $date, $form_state);
}

/**
 * Convert jQuery Datepicker date format to PHP. PHP format is more advanced
 * so the reverse operation is not possible easily.
 *
 * @param string $format
 *   jQuery date picker date format
 * @param boolean $from
 *   If set to false, the format will be converted from PHP to jQuery date
 *   picker, but this is unstable and may have random behavior because of
 *   literals and feature differences between the 2 formats.
 *
 * @return string
 */
function udate_format_convert($format, $from = true) {
  // $.datepicker.CONSTANT constants.
  $const = array(
    'ATOM'     => "yy-mm-dd",
    'COOKIE'   => "D, dd M y",
    'ISO_8601' => "yy-mm-dd",
    'RFC_822'  => "D, d M y",
    'RFC_850'  => "DD, dd-M-y",
    'RFC_1036' => "D, d M y",
    'RFC_1123' => "D, d M yy",
    'RFC_2822' => "D, d M yy",
    'RSS'      => "D, d M y",
    'W3C'      => "yy-mm-dd",
  );
  // Conversion matrix. Order is important, "mm" must be before "m" in order
  // to avoid "mm" to be converted to "nn" while it should be just "m".
  $toConv = array(
    'd' => 'dd',
    'j' => 'd',
    'l' => 'DD',
    'D' => 'D',
    'm' => 'mm',
    'n' => 'm',
    'F' => 'MM',
    'M' => 'M',
    'Y' => 'yy',
    'y' => 'y',
  );
  $fromConv = array_flip($toConv);

  if ($from) {
    if (isset($const[$format])) {
      $format = $const[$format];
    }
    // FIXME: Missing litterals that could conflict with PHP ones.
    return strtr($format, $fromConv);
  } else {
    return strtr($format, $toConv);
  }
}

