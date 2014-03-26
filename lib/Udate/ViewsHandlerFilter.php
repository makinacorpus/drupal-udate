<?php

/**
 * Views argument handler for dates.
 */
class Udate_ViewsHandlerFilter extends views_handler_filter_date
{
    /**
     * @see date_views_handler_is_date()
     */
    public $min_date = true;

    public function option_definition()
    {
        $options = parent::option_definition();
        $options['granularity'] = array('default' => UDATE_GRANULARITY_SECOND);

        return $options;
    }

    public function extra_options_form(&$form, &$form_state)
    {
        parent::extra_options_form($form, $form_state);

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
