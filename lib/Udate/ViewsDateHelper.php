<?php

/**
 * Set of utility functions for SQL dates.
 */
class Udate_ViewsDateHelper
{
    /**
     * Build SQL formula
     *
     * @param DateTime $date
     * @param int $granularity
     */
    static public function getBounds(DateTime $date, $granularity = UDATE_GRANULARITY_SECOND)
    {
        $bounds = array(clone $date, clone $date);

        switch ($granularity) {

            case UDATE_GRANULARITY_SECOND:
              return array($date, $date);

            case UDATE_GRANULARITY_MINUTE:
              $bounds[0]->setDate($date->get, $month, $day); 
              return array(
                  DateTime::createFromFormat(UDATE_PHP_DATETIME, $date->format("Y-m-d H:i:00")),
                  DateTime::createFromFormat(UDATE_PHP_DATETIME, $date->format("Y-m-d H:i:59")),
              );

            case UDATE_GRANULARITY_HOUR:
              return array(
                  DateTime::createFromFormat(UDATE_PHP_DATETIME, $date->format("Y-m-d H:00:00")),
                  DateTime::createFromFormat(UDATE_PHP_DATETIME, $date->format("Y-m-d H:59:59")),
              );

            case UDATE_GRANULARITY_DAY:
              return array(
                  DateTime::createFromFormat(UDATE_PHP_DATETIME, $date->format("Y-m-d 00:00:00")),
                  DateTime::createFromFormat(UDATE_PHP_DATETIME, $date->format("Y-m-d 23:59:59")),
              );

            case UDATE_GRANULARITY_MONTH:
              return array(
                  DateTime::createFromFormat(UDATE_PHP_DATETIME, $date->format("Y-m-01 00:00:00")),
                  DateTime::createFromFormat(UDATE_PHP_DATETIME, $date->format("Y-m-31 23:59:59")),
              );

            case UDATE_GRANULARITY_YEAR:
              return array(
                  DateTime::createFromFormat(UDATE_PHP_DATETIME, $date->format("Y-01-01 00:00:00")),
                  DateTime::createFromFormat(UDATE_PHP_DATETIME, $date->format("Y-12-31 23:59:59")),
              );

            default:
              return null;
        }
    }
}
