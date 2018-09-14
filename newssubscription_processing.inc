<?php

/**
 * Defines e-mail.
 */
function newssubscription_get_confirmation_mail(&$message, $params) {
  if (!$params['hash'] || !$params['form_state']) {
    throw new \Exception('Params hash or form_state missing.');
  }

  $table = newssubscription_overview_table($params['form_state']);

  global $base_url;
  $validate_url = $base_url . '/nieuwsaanmelding/bevestig/' . $params['hash'];

  $message['subject'] = 'Bevestig je aanmelding voor SP nieuws';
  $message['body'][] = '<p>Beste ' . $params['first_name'] . ',</p>';
  $message['body'][] = '<p>Bedankt voor je aanmelding voor SP nieuws!<br />Klik op onderstaande link om je aanmelding te bevestigen. Als je niets doet wordt je aanmelding vanzelf verwijderd.</p>';
  $message['body'][] = '<p>' . l($validate_url, $validate_url) . '</p>';
  $message['body'][] = $table;
  $message['body'][] = '<p>Met vriendelijke groet,<br />SP';
}

/**
 * Defines reminder mail.
 */
function newssubscription_get_reminder_mail(&$message, $params) {

  if (!$params['hash']) {
    throw new \Exception('Params hash missing.');
  }

  global $base_url;
  $validate_url = $base_url . '/nieuwsaanmelding/bevestig/' . $params['hash'];

  $message['subject'] = 'Herinnering: bevestig je aanmelding voor SP nieuws';
  $message['body'][] = '<p>Beste ' . $params['first_name'] . ',</p>';
  $message['body'][] = '<p>Gisteren heb je je aangemeld voor SP nieuws.<br />Je hebt je aanmelding echter nog niet bevestigd. Klik op onderstaande link om je aanmelding te bevestigen.</p>';
  $message['body'][] = '<p>' . l($validate_url, $validate_url) . '</p>';
  $message['body'][] = '<p>Met vriendelijke groet,<br />SP';
}

/**
 * Defines e-mail.
 */
function newssubscription_get_success_mail(&$message, $params) {

  $message['subject'] = 'Je aanmelding voor SP nieuws is gelukt!';
  $message['body'][] = '<p>Beste ' . $params['firstname'] . ',</p>';
  $message['body'][] = '<p>Bedankt voor je aanmelding voor SP nieuws!.';
  $message['body'][] = '<p>Vanaf nu wordt je automatisch op de hoogte gehouden.</p>';
  $message['body'][] = '<p>Met vriendelijke groet,<br />SP';
}

function newssubscription_submit_pending() {

  $pending = db_select('newssubscription_submissions', 's')
    ->fields('s')
    ->isNotNULL('verified')
    ->isNULL('submitted')
    ->execute();

  if ($pending) {
    while ($submission = $pending->fetchAssoc()) {
      newssubscription_subscribe($submission);
    }
  }
  return TRUE;
}

function newssubscription_subscribe($submission = []) {
  if (!empty($submission['name']) && !empty($submission['email'])) {
    $sync_data = array(
      'name' => $submission['name'],
      'email' => $submission['email'],
    );

    // Create contact source line.
    $site_name = variable_get('site_name');
    global $base_url;
    $parsed_url = parse_url($base_url);
    $sync_data['source'] = 'Form: Nieuwsaanmelding (sid: ' . $submission['id'] . ', ' .$parsed_url['host'] . ')';

    $result = spcivipush_push_contact($sync_data, $submission);
    $error = newssubscription_push_contact_process_result($result, $submission, $sync_data);
    if ($error) {
      $state = $error;
      watchdog('newssubscription', 'Error processing submission ' . $submission['id'] . ' (' . $result . ').');
    }
    else {
      $contact_id = $result;
      // Add subscription information to contact.
      $error = newssubscription_push_subscriptions($contact_id, $submission);
      if ($error) {
        $state = 'error';
        watchdog('newssubscription', 'Error processing submission ' . $submission['id'] . '.');
      }
      else {
        // TODO: Send emails.
        $mail = drupal_mail(
          'newssubscription', 'success', $submission['email'], LANGUAGE_NONE, $submission);
        //spwebformsync_send_mails($contact_id, $sync_data, $submission, $webform);
        $state = 'synced';
        watchdog('newssubscription', 'Processed submission ' . $submission['id']);
      }
    }
    newssubscription_update_submission($submission['id'], $state, $contact_id);
  }
}

function newssubscription_push_contact_process_result($result, $submission, $sync_data) {
  if (in_array($result, array('error', 'insufficient data', 'contact id not found'))) {
    $contact_id = NULL;
  }
  else {
    $contact_id = $result;
    $result = 'synced';
  }
  return ($result !== 'synced');
}

function newssubscription_update_submission($sid, $state, $contact_id = NULL) {
  $states = array(
    'unprocessed' => '0',
    'synced' => '1',
    'error' => '2',
    'insufficient data' => '3',
    'contact id not found' => '4',
  );
  if (!empty($sid) && isset($states[$state])) {
    $new_state = $states[$state];
    // Check if state exists.
    db_merge('newssubscription_submissions')
    ->key(array('id' => $sid))
    ->fields(array(
      'submitted' => time(),
      'pushstate' => $new_state,
      'contact_id' => $contact_id,
    ))
    ->execute();
  }
  return FALSE;
}

function newssubscription_push_subscriptions($contact_id, $subscription) {
  if (!empty($contact_id) && !empty($subscription)) {

    $spcivi = \SPCivi::getInstance();

    // Push subscribed themes.
    $themes = array_keys(newssubscription_get_themes());
    $subscribed_themes = array();
    foreach ($themes as $theme) {
      if ($subscription[$theme]) {
        $subscribed_themes[] = $theme;
      }
    }

    $query = array(
      'sequential' => 1,
      'id' => $contact_id,
      'custom_' . variable_get('newssubscription_newsletter_field') => $subscription['newsletter'] == 0 ? '' : 1,
      'custom_' . variable_get('newssubscription_themes_field') => $subscribed_themes,
      'custom_' . variable_get('newssubscription_frequency_field') => $subscription['frequency'],
    );
    $result = $spcivi->api('Contact', 'create', $query);
    if (isset($result['is_error']) && $result['is_error'] === 0) {
      return FALSE;
    }
  }
  return TRUE;
}

/**
 * Returns themes.
 */
function newssubscription_get_themes() {
  return array(
    'zorgwelzijnsport' => array(
      'title' => 'Zorg, Welzijn en Sport',
      'nid' => 11,
    ),
    'werkinkomen' => array(
      'title' => 'Werk en Inkomen',
      'nid' => 16,
    ),
    'wonen' => array(
      'title' => 'Wonen',
      'nid' => 15,
    ),
    'onderwijscultuur' => array(
      'title' => 'Onderwijs en Cultuur',
      'nid' => 17,
    ),
    'economie' => array(
      'title' => 'Economie',
      'nid' => 31,
    ),
    'rechtveiligheid' => array(
      'title' => 'Recht en Veiligheid',
      'nid' => 29,
    ),
    'natuurmilieu' => array(
      'title' => 'Natuur en Milieu',
      'nid' => 28,
    ),
    'internationalezaken' => array(
      'title' => 'Internationale zaken',
      'nid' => 32,
    ),
    'nederlandeuropa' => array(
      'title' => 'Nederland en Europa',
      'nid' => 14,
    ),
    'belastingfinancien' => array(
      'title' => 'Belasting en Financiën',
      'nid' => 21,
    ),
    'verkeer' => array(
      'title' => 'Verkeer',
      'nid' => 187,
    ),
    'overheidbestuur' => array(
      'title' => 'Overheid en Bestuur',
      'nid' => 30,
    ),
    'defensie' => array(
      'title' => 'Defensie',
      'nid' => 24,
    ),
    'organisatiesp' => array(
      'title' => 'Organisatie SP',
      'nid' => 189,
    ),
  );
}

function newssubscription_get_themes_list() {
  $themes_list = &drupal_static(__FUNCTION__);
  if (!isset($themes_list)) {
    $themes_list = array();
    $themes = newssubscription_get_themes();
    foreach($themes as $key => $value) {
      $themes_list[$key] = $value['title'];
    }
  }
  return $themes_list;
}

function newssubscription_get_themes_nids() {
  $themes_nids = &drupal_static(__FUNCTION__);
  if (!isset($themes_nids)) {
    $themes_nids = array();
    $themes = newssubscription_get_themes();
    foreach($themes as $key => $value) {
      $themes_nids[$key] = $value['nid'];
    }
  }
  return $themes_nids;
}

function newssubscription_process_civi_result($result, $error_message) {
  if (!isset($result['is_error']) || $result['is_error'] == 1) {
    watchdog('newssubscription', $error_message);
    drupal_set_message($error_message, 'error');
    return FALSE;
  }
  return TRUE;
}
