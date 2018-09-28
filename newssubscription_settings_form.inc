<?php

function newssubscription_settings_form($form, &$form_state) {
  // Add custom submit function.
  $form['#submit'][] = 'newssubscription_settings_form_submit';

  // General settings.
  $form['newssubscription_general_settings'] = array(
    '#type' => 'fieldset',
    '#title' => 'Nieuwsaanmedling algemene instellingen',
  );

  $form['newssubscription_general_settings']['newssubscription_active'] = array(
    '#type' => 'checkbox',
    '#title' => 'Verzend nieuwsupdates',
    '#default_value' => variable_get('newssubscription_active', NULL),
  );

  $image_styles = image_style_options();
  $form['newssubscription_general_settings']['newssubscription_thumb_image_style'] = array(
    '#type' => 'select',
    '#title' => 'Kies de afbeeldingsstijl voor de nieuwsberichten',
    '#options' => $image_styles,
    '#default_value' => variable_get('newssubscription_thumb_image_style', 'thumbnail'),
    '#required' => TRUE,
  );

  $secret_default = variable_get('newssubscription_secret', NULL);
  $form['newssubscription_general_settings']['newssubscription_secret'] = array(
    '#type' => 'textfield',
    '#title' => 'Nieuwsaanmelding secret',
    '#description' => 'Wachtwoord dat wordt gebruikt om uitschrijf link te encoden / decoden.',
    '#default_value' => $secret_default,
  );

  if (!empty($secret_default)) {
    $form['newssubscription_general_settings']['newssubscription_secret']['#attributes']['disabled'] = TRUE;
  }

  // Civicrm settings.
  $form['newssubscription_civicrm_settings'] = array(
    '#type' => 'fieldset',
    '#title' => 'Nieuwsaanmedling CiviCRM velden',
  );

  $custom_fields = newssubscription_get_civi_custom_fields();

  if ($custom_fields) {
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
  }
  else {
    $form['civicrm_error'] = array(
      '#markup' => '<p>Er kan geen verbinding met CiviCRM worden gemaakt. Controleer de CiviCRM API instellingen.</p>',
    );
  }

  return system_settings_form($form);
}

function newssubscription_settings_form_submit($form, &$form_state) {
  $values = $form_state['values'];
  if ($form['newssubscription_general_settings']['newssubscription_active']['#default_value'] !== $values['newssubscription_active']) {
    if ($values['newssubscription_active']) {
      variable_set('newssubscription_active_date', time());
    }
  }
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
