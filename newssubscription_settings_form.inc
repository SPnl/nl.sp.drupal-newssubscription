<?php

function newssubscription_settings_form($form, &$form_state) {

  // Civicrm settings.
  $form['newssubscription_civicrm_settings'] = array(
    '#type' => 'fieldset',
    '#title' => 'Nieuwsaanmedling CiviCRM velden',
  );

  $custom_fields = newssubscription_get_civi_custom_fields();

  $form['newssubscription_civicrm_settings']['newssubscription_newsletter_field'] = array(
    '#type' => 'select',
    '#title' => "Nieuwsbrief veld",
    '#options' => $custom_fields,
    '#empty_option' => '- Geen -',
    '#default_value' => variable_get('newssubscription_newsletter_field', array()),
  );
  $form['newssubscription_civicrm_settings']['newssubscription_themes_field'] = array(
    '#type' => 'select',
    '#title' => "Thema's veld:",
    '#options' => $custom_fields,
    '#empty_option' => '- Geen -',
    '#default_value' => variable_get('newssubscription_themes_field', array()),
  );

  $form['newssubscription_civicrm_settings']['newssubscription_frequency_field'] = array(
    '#type' => 'select',
    '#title' => "Nieuwsmailfrequentie veld",
    '#options' => $custom_fields,
    '#empty_option' => '- Geen -',
    '#default_value' => variable_get('newssubscription_frequency_field', array()),
  );

  return system_settings_form($form);
}

function newssubscription_get_civi_custom_fields() {
  $custom_fields = &drupal_static(__FUNCTION__);
  if (!isset($custom_fields)) {

    $custom_fields = array();
    set_time_limit(0);
    $spcivi = \SPCivi::getInstance();
    $params = array(
      'sequential'            => 1,
      'option.limit'          => 9999,
      'is_active' => 1,
    );
    $result = $spcivi->api('CustomField', 'get', $params);
    if (!isset($result['is_error']) || $result['is_error'] == 1) {
      watchdog('newssubscription', 'Fout bij het ophalen van custom fields uit CiviCRM.');
      drupal_set_message('Fout bij het ophalen van custom fields uit CiviCRM', 'error');
      return FALSE;
    }

    if (!empty($result['values'])) {
      foreach ($result['values'] as $value) {
        $custom_fields[$value['id']] = $value['label'] . ' (' . $value['id'] . ')';
      }
      asort($custom_fields);
    }
  }
  return $custom_fields;
}
