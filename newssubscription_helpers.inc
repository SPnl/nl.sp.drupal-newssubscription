<?php

/**
 * @file Helperfunctions for newssubscription module.
 */

/**
 * Removes error messages, and stores errors in form state.
 */
function newssubscription_remove_errors(&$form_state) {
  // Remove form error messages.
  if (!empty($form_state['form_errors'])) {
    $errors = $form_state['form_errors'];
    $error_messages = &$_SESSION['messages']['error'];
    if (!empty($error_messages)) {
      foreach ($errors as $error) {
        if (($key = array_search($error, $error_messages)) !== FALSE) {
          unset($error_messages[$key]);
          if (empty($error_messages)) {
            unset($_SESSION['messages']['error']);
          }
        }
      }
    }
    // Store errors for display in form elements.
    $form_state['form_errors'] = $errors;
    // Get the array of errors and reset them so form will be submitted.
    $errors = &drupal_static("form_set_error", array());
    // Make sure that each error should be applied to the field.
    foreach (array_keys($errors) as $field) {
      unset($errors[$field]);
    }
  }
}

/**
 * Gets stored form data from database.
 *
 * @param int $sid
 *   Form submission id.
 *
 * @return array $storage
 *   Stored form data.
 */
function newssubscription_get_storage($sid = NULL, $form_name = '') {
  $storage = array();
  if (!isset($sid) && isset($_SESSION['newssubscription'][$form_name]['sid'])) {
    $sid = $_SESSION['newssubscription'][$form_name]['sid'];
  }

  if (isset($sid)) {
    // Get storage from database if available.
    $query = "SELECT form_values FROM {newssubscription_storage} WHERE sid = :sid";
    $result = db_query($query, array(':sid' => $sid))->fetchAssoc();
    if (!empty($result['form_values'])) {
      $storage = unserialize($result['form_values']);
      $storage['sid'] = $sid;
    }
  }
  return $storage;
}

/**
 * Stores database storage variable in database.
 *
 * @param array $storage
 *   Database storage variable.
 */
function newssubscription_store_form_data(array &$storage = NULL) {
  if (isset($storage['sid'])) {
    // Update database storage.
    db_merge('newssubscription_storage')
    ->key(array('sid' => $storage['sid']))
    ->fields(array(
      'form_values' => serialize($storage),
    ))
    ->execute();
  }
}

/**
 * Return the names of the different forms defined in /etc/pages.
 *
 * @return array
 *   List of pagenames.
 */
function newssubscription_get_formnames() {
  $paths = glob(drupal_get_path('module', 'newssubscription') . '/multipage_forms/*');
  foreach ($paths as $path) {
    $result = preg_match('/^.*\/newssubscription_([^.]*)_pages.inc$/', $path, $matches);
    if (!empty($matches[1])) {
      $form_names[] = $matches[1];
    }
  }
  return $form_names;
}

/**
 * Defines e-mail.
 */
function newssubscription_get_confirmation_mail(&$message, $params) {
  dpm('TEST');

  if (!$params['hash'] || !$params['form_state']) {
    throw new \Exception('Params hash or form_state missing.');
  }

  $table = newssubscription_overview_table($params['form_state']);

  global $base_url;
  $validate_url = $base_url . '/nieuwsaanmelding/bevestig/' . $params['hash'];

  $message['subject'] = 'Bevestig je aanmelding voor SP nieuws';
  $message['body'] = [];
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
  $message['body'] = [];
  $message['body'][] = '<p>Gisteren heb je je aangemeld voor SP nieuws.<br />Je hebt je aanmelding echter nog niet bevestigd. Klik op onderstaande link om je aanmelding te bevestigen.</p>';
  $message['body'][] = '<p>' . l($validate_url, $validate_url) . '</p>';
  $message['body'][] = '<p>Met vriendelijke groet,<br />SP';
}

function newssubscription_submit_pending() {

  $pending = db_select('newssubscription_submissions', 's')
    ->fields('s')
    ->isNotNULL('verified')
    ->isNULL('submitted')
    ->execute();

  if ($pending) {
    while ($record = $pending->fetchAssoc()) {

      try {
        $contact_id = newssubscription_subscribe($record);
        $error = NULL;
      }
      catch (\Exception $e) {
        $error = $e->getMessage();
        $contact_id = NULL;
      }

      db_update('newssubscription_submissions')
        ->fields([
          'submitted'  => time(),
          'contact_id' => $contact_id,
        ])
        ->condition('id', $record['id'])
        ->execute();
    }
  }
  return TRUE;
}

function newssubscription_subscribe($data = []) {
}
