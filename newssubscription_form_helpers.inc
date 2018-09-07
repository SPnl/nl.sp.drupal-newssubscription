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

function newssubscription_clear_form(&$form_state) {
  if (!empty($form_state['storage']['sid'])) {
    $number = db_delete('newssubscription_storage')
    ->condition('sid', $form_state['storage']['sid'])
    ->execute();
  }
  unset($_SESSION['newssubscription']);
  //unset($form_state['storage']); Removed because it breaks google analytics in ajax callback.
}

function newssubscription_modify_form_elements(&$item, &$form_state, $key) {
  $item_org = $item;
  $storage = &$form_state['storage'];
  if (is_array($item)) {
    if (preg_match('/^page_' . $storage['current_page'] . '_[a-z0-9_]+$/', $key, $matches)) {
      // Add default values.
      if (isset($storage['values'][$storage['current_page']][$key])) {
        $item['#default_value'] = $storage['values'][$storage['current_page']][$key];
      }
      else {
        if (empty($item['#default_value'])) {
          unset($item['#default_value']);
        }
      }
      // Add error messages.
      if (isset($form_state['form_errors'][$key])) {
        $item['#prefix'] = '<div class="newssubscription validation-error">';
        $item['#suffix'] = '<div class="messages error">' . $form_state['form_errors'][$key] . '</div></div>';
      }
      else {
        $item['#prefix'] = '<div class="newssubscription">';
        $item['#suffix'] = '</div>';
      }
      // Rebuild required functionality.
      if (!empty($item['#required'])) {
        unset($item['#required']);
        $item['#title'] .= ' <span class="form-required" title="This field is required.">*</span>';
        $storage['required_fields'][$key] = $key;
      }
      // Store form definition for use in other pages
      // (review page for example).
      $storage['form_element_definitions'][$key] = $item;
    }
    else {
      foreach ($item as $key2 => $item2) {
        newssubscription_modify_form_elements($item[$key2], $form_state, $key2);
      }
    }
  }
}

function newssubscription_get_nav($storage) {
  if (!empty($storage['current_page'])) {
    $current_nav_history_key = array_search($storage['current_page'], $storage['nav_history']);
    if ($current_nav_history_key !== FALSE) {
      $prev_elements = array_reverse(array_slice($storage['nav_history'], 0, $current_nav_history_key, TRUE));
      $previous_page = false;
      foreach($prev_elements as $key => $page) {
        if (array_search($page, $storage['no_nav_pages']) === FALSE) {
          $previous_page = $page;
          break;
        }
      }
      $next_elements = array_slice($storage['nav_history'], $current_nav_history_key + 1, NULL, TRUE);
      $next_page = false;
      foreach($next_elements as $key => $page) {
        if (array_search($page, $storage['no_nav_pages']) === FALSE) {
          $next_page = $page;
          break;
        }
      }
      $result = array(
        'previous_page' => $previous_page,
        'current_page' => $storage['current_page'],
        'next_page' => $next_page,
      );
      return $result;
    }
  }
  return FALSE;
}
