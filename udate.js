/*jslint browser: true, indent: 2 */
/*global jQuery, Drupal */
(function ($) {
  "use strict";
  Drupal.behaviors.udate = {
    attach: function (context) {
      var id, settings;
      if (!Drupal.settings.udate) {
        return; // Failsafe.
      }
      for (id in Drupal.settings.udate) {
        settings = Drupal.settings.udate[id];
        if ("object" === typeof settings && settings.dateFormat) {
          $(context).find('#' + id).once('udate', function () {
            $(this).datepicker(settings);
          });
        }
      }
    }
  };
}(jQuery));