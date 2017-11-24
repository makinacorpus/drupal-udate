/*jslint browser: true, indent: 2 */
/*global jQuery, Drupal */
(function ($) {
  "use strict";

  function checkDateInAllowedRanges(date, ranges) {
    var formatted = $.datepicker.formatDate('mm-dd', date);
    for (var i in ranges) {
      var current = ranges[i];
      if (current[0] <= formatted && formatted <= current[1]) {
        return [true];
      }
    }
    return [false];
  }

  Drupal.behaviors.udate = {
    attach: function (context) {
      var id, settings;
      if (!Drupal.settings.udate) {
        return; // Failsafe.
      }
      for (id in Drupal.settings.udate) {
        settings = Drupal.settings.udate[id];
        if ("object" === typeof settings && settings.dateFormat) {

          if ((settings.allowedRanges instanceof Array) && settings.allowedRanges.length) {
            // jQuery.datepicker seems to drop the unknown options so we must
            // keep a copy here.
            var ranges = settings.allowedRanges;
            settings.beforeShowDay = function (date) {
              return checkDateInAllowedRanges(date, ranges);
            };
          }

          $(context).find('#' + id).once('udate', function () {
            $(this).datepicker(settings);
          });
        }
      }
    }
  };
}(jQuery));