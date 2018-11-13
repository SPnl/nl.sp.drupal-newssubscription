<?php

/**
 * @file Form definitions for newssubscription pages.
 *
 *   Define page form functions like 'newssubscription_x_page_x_form' where the
 *   first x is to be substituted with the form name and the second x with the
 *   page name used in $storage['current_page'] and $storage['next_page'].
 *
 *   Example:
 *
 *   function newssubscription_subscribe_start_form(&$form, $storage, $values) {
 *     $form['page_start_choice'] = array(
 *       '#type' => 'radios',
 *       '#title' => 'Maak een keuze',
 *       '#options' => array('colors' => 'Kleuren', 'tastes' => 'Smaken'),
 *       '#required' => TRUE,
 *     );
 *   }
 */

/**
 * Form settings.
 *
 * Add 'no_nav_pages' variable (array) with page names of pages that should not
 * show a back button.
 *
 * Add 'pages' array with page name to page titles.
 *
 * @param string $variable_name
 *   Name of the variable to return.
 *
 * @return Variable value.
 *   Returns variable value.
 */
function newssubscription_subscribe_settings($variable_name) {
  $settings = array(
    'path_name' => 'nieuwsaanmelding/aanmelden',
    'path_arguments' => array(
      'hash',
    ),
    'form_title' => 'Aanmelden voor nieuws',
    'no_nav_pages' => array(
      'thanks',
    ),
    'pages' => array(
      'start' => 'Contactgegevens',
      'select' => "Nieuwsselectie",
      'summary' => 'Controleer uw aanmelding',
      'thanks' => 'Bedankt',
    ),
    'final_submit_page' => 'summary',
    'feedback_page' => 'thanks',
  );
  if (isset($settings[$variable_name])) {
    return $settings[$variable_name];
  }
}

/**
 * Form definition for start page.
 */
function newssubscription_subscribe_page_start_form(&$form, &$form_state, $arg1 = NULL, $arg2 = NULL, $arg3 = NULL, $arg4 = NULL) {
  // Check if settigns are ok.
  if (
    empty(variable_get('newssubscription_newsletter_field')) ||
    empty(variable_get('newssubscription_themes_field')) ||
    empty(variable_get('newssubscription_frequency_field'))
  ) {
    $form['settings_error'] = array(
      '#markup' => '<p>Voor een juiste werking van de nieuwsaanmeldmodule moeten eerst de instellingen voor CiviCRM goed gezet worden. Ga hiervoor als beheerder naar <a href="/admin/config/sp/nieuwsaanmeldingen/instellingen">nieuwsaanmelding instellingen</a> en selecteer de correcte CiviCRM velden. Zodra de juiste CiviCRM velden zijn geselecteerd werkt het aanmeldformulier.</p>',
    );
    return;
  }

  // Create form.
  $form['input'] = array(
    '#type' => 'fieldset',
    '#attributes' => array(
      'class' => array('newssubscriptionform-input'),
    ),
  );

  $form['input']['page_start_name'] = array(
    '#type' => 'textfield',
    '#title' => 'Naam',
    '#required' => TRUE,
    '#attributes' => array('placeholder' => 'Voornaam Achternaam'),
  );

  $form['input']['page_start_email'] = array(
    '#type' => 'emailfield',
    '#title' => 'E-mailadres',
    '#required' => TRUE,
    '#attributes' => array('placeholder' => 'naam@domein.nl'),
  );
}

/**
 * Form definition for thanks phone page.
 */
function newssubscription_subscribe_page_select_form(&$form, &$form_state) {
  $storage = &$form_state['storage'];

  $form['input'] = array(
    '#type' => 'fieldset',
    '#attributes' => array(
      'class' => array('newssubscriptionform-input'),
    ),
  );

  $form['input']['page_select_newsletter'] = array(
    '#type' => 'checkboxes',
    '#title' => 'Belangrijk SP nieuws',
    '#options' => array('newsletter' => 'Ik ontvang graag de SP nieuwsbrief met belangrijk SP nieuws.'),
    '#default_value' => array('newsletter'),
  );

  $themes = newssubscription_get_themes_list();

  $form['input']['page_select_themes'] = array(
    '#type' => 'checkboxes',
    '#title' => "Ik ontvang graag nieuws over de volgende thema's",
    '#options' => $themes,
    '#ajax' => array(
      'wrapper' => 'newssubscription_form_wrapper',
      'callback' => 'newssubscription_ajax_callback',
    ),
    '#default_value' => array_keys($themes),
  );

  $frequenties = array(
    'direct' => 'Direct',
    'dagelijks' => 'Dagelijks overzicht',
    'wekelijks' => 'Wekelijks overzicht',
  );

  if (!empty($storage['values']['select']['page_select_themes'])) {
    $theme_selected = FALSE;
    foreach ($storage['values']['select']['page_select_themes'] as $key => $val) {
      if ($key === $val) {
        $theme_selected = TRUE;
        break;
      }
    }
    if ($theme_selected) {
      $form['input']['page_select_frequence'] = array(
        '#type' => 'radios',
        '#title' => "Ontvangst thematisch nieuws",
        '#options' => $frequenties,
        '#required' => TRUE,
      );
    }
  }
}

/**
 * Form definition for summary page.
 */
function newssubscription_subscribe_page_summary_form(&$form, &$form_state) {
  $markup = newssubscription_overview_table($form_state);
  $form['overview'] = array(
    '#markup' => $markup,
  );
}

/**
 * Form definition for thanks page.
 */
function newssubscription_membership_page_thanks_form(&$form, &$form_state) {
  $form['intro'] = array(
    '#markup' => "<div class=\"introduction\"><p>TEST</p></div>",
  );
}

/**
 * Validation for member form pages.
 */
function newssubscription_subscribe_page_form_validate($form, &$form_state) {
  $values = &$form_state['values'];
  $storage = &$form_state['storage'];
  // Process trigger.
  $trigger = $form_state['triggering_element']['#name'];
  if ($form_state['validate']) {

    switch ($storage['current_page']) {
      case 'start':
        // Validate email.
        if (!empty($values['page_start_email'])) {
          if (!valid_email_address($values['page_start_email'])) {
            form_set_error('page_start_email', 'Geef een geldig e-mailadres op!');
          }
        }
        // Validate name.
        $words_in_name = str_word_count($values['page_start_name'], 1);
        foreach ($words_in_name as $word) {
          if (strlen($word) < 2) {
            form_set_error('page_start_name', 'Vul uw volledige naam in, en niet slechts uw voorletter(s)');
          }
        }
        break;

      case 'select':
        foreach ($values['page_select_themes'] as $key => $val) {
          if ($key === $val) {
            $list[$key] = $val;
          }
        }
        if (empty($list) && empty($values['page_select_newsletter']['newsletter'])) {
          form_set_error('page_select_themes', 'Kies tenminste één nieuwsaanmelding');
        }
        break;
    }
  }
}

/**
 * Submit function for newssubscription pages.
 *
 * Set $storage['next_page'] to value of next page.
 */
function newssubscription_subscribe_page_form_submit($form, &$form_state) {
  $values = &$form_state['values'];
  $storage = &$form_state['storage'];

  // Process trigger.
  $trigger = $form_state['triggering_element']['#name'];

  switch ($storage['current_page']) {
    case 'start':
      // Set form navigation.
      $storage['next_page'] = 'select';
      // Store hash name, value and created.
      $split_name = newssubscription_split_name($values['page_start_name']);
      $storage['first_name'] = $split_name['first_name'];
      $storage['middle_name'] = $split_name['middle_name'];
      $storage['last_name'] = $split_name['last_name'];
      break;

    case 'select':
      $storage['next_page'] = 'summary';
      break;

    case 'summary':
      $storage['next_page'] = 'thanks';
      // Store submission.
      $storage['hash'] = empty($storage['hash']) ? newssubscription_get_hash('newssubscription', $storage['values']['start']['page_start_name']) : $storage['hash'];
      $storage['created'] = empty($storage['created']) ? time() : $storage['created'];
      $store_result = newssubscription_store_submission($form_state);

      if (!$store_result['submission_id']) {
        $form_state['redirect'] = array('nieuwsaanmelding/fout');
      }

      $fields = $store_result['fields'];
      if ($storage['current_page'] == 'summary' && !empty($fields['email']) && $trigger === 'next_page') {
        // Send confirmation mail when submitting summary page.
        $mail = drupal_mail(
          'newssubscription', 'verificatie', $fields['email'], LANGUAGE_NONE,
          array(
            'first_name'      => $storage['first_name'],
            'form_state'      => $form_state,
            'hash'            => $storage['hash'],
            'format'          => SWIFTMAILER_FORMAT_HTML,
          )
        );
        if (empty($mail['send'])) {
          $storage['next_page'] = $storage['current_page'];
          drupal_set_message('Het is niet gelukt om de bevestigingsmail te versturen. Houdt dit probleem aan, neem dan contact op met webmaster@sp.nl', 'error');
        }
      }
      break;
  }
}
