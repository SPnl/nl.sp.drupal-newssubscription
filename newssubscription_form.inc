<?php

/**
 * @file
 * Memberform definition and validation and submit functions.
 */

/**
 * Form constructor for the member form.
 */
function newssubscription_form($form, &$form_state, $form_name) {
  $values = isset($form_state['values']) ? $form_state['values'] : array();

  $sid = empty($form_state['storage']['sid']) ? NULL : $form_state['storage']['sid'];
  $storage = newssubscription_get_storage($sid, $form_name);

  // Store storage data in form.
  $form_state['storage'] = $storage;
  $storage = &$form_state['storage'];

  // Store form name.
  $storage['form_name'] = $form_name;

  // Store settings.
  $settings_callback = 'newssubscription_' . $form_name . '_settings';
  $storage['pages'] = $settings_callback('pages');
  $storage['no_nav_pages'] = $settings_callback('no_nav_pages');
  $storage['final_submit_page'] = $settings_callback('final_submit_page');
  $storage['feedback_page'] = $settings_callback('feedback_page');

  // Set default page.
  if (empty($storage['current_page'])) {
    $storage['current_page'] = 'start';
    $storage['nav_history'][] = $storage['current_page'];
  }

  $form_state['validate'] = TRUE;

  // Create form based on form page.
  $form = array(
    '#validate' => array(
      // This order is needed because of required elements inline message.
      'newssubscription_form_pre_custom_validate',
      'newssubscription_' . $form_name . '_page_form_validate',
      'newssubscription_form_post_custom_validate',
    ),
    '#submit' => array(
      'newssubscription_' . $form_name . '_page_form_submit',
      'newssubscription_form_submit',
    ),
    '#prefix' => '<div id="newssubscription_form_wrapper">',
    '#suffix' => '</div>',
    '#attached' => array(
      'css' => array(
        drupal_get_path('module', 'newssubscription') . '/css/newssubscription.css',
      ),
      'js' => array(
        drupal_get_path('module', 'newssubscription') . '/js/newssubscription.js',
      ),
    ),
    '#attributes' => array('class' => array('newssubscription_' . $form_name)),
  );

  // Add header image.
  $file_path = drupal_get_path('module', 'newssubscription') . '/header_images/' . $storage['current_page'] . '.jpg';
  if (file_exists($file_path)) {
    global $base_url;
    $form['header'] = array(
      '#markup' => '<div class="form_header"><img src="' . $base_url . '/' . drupal_get_path('module', 'newssubscription') . '/header_images/' . $storage['current_page'] . '.jpg" /></div>',
    );
  }

  // Add form navigation.
  if ($storage['current_page'] !== $storage['feedback_page'] && !in_array($storage['current_page'], $storage['no_nav_pages'])) {
    $form['form_navigation'] = array(
      '#type' => 'fieldset',
      '#attributes' => array('class' => array('newssubscription-navigation')),
    );
    $count = 0;
    $page_names_short = array(
      'start' => 'Contactgegevens',
      'select' => 'Nieuwsselectie',
      'summary' => 'Controle',
    );

    foreach($storage['pages'] as $form_page => $title) {
      if (!in_array($form_page, $storage['no_nav_pages'])) {
        $title_short = $page_names_short[$form_page];
        $count++;
        $storage['navigation_order'][$count] = $form_page;
        $form['form_navigation']['go_to_' . $form_page] = array(
          '#type' => 'submit',
          '#value' => $title_short,
          '#name' => 'go_to_' . $form_page,
          '#ajax' => array(
            'wrapper' => 'newssubscription_form_wrapper',
            'callback' => 'newssubscription_ajax_callback',
          ),
          '#disabled' => !in_array($form_page, $storage['nav_history']) || $form_page === $storage['current_page'],
          '#attributes' => ($form_page === $storage['current_page']) ? array('class' => array('active')) : array(),
        );
      }
    }
  }

  // Add form title.
  if (!empty($storage['pages'][$storage['current_page']])) {
    $form['progress'] = array(
      '#markup' => '<h3>' . $storage['pages'][$storage['current_page']] . '</h3>',
    );
  }

  // Get pages forms.
  $form_callback = 'newssubscription_' . $form_name . '_page_' . $storage['current_page'] . '_form';
  if (function_exists($form_callback)) {

    // Add page specific form elements.
    $form_callback($form, $form_state);

    // Modify form elements (Add default values, error messages, rebuild required functionality).
    foreach ($form as $key => $item) {
      newssubscription_modify_form_elements($form[$key], $form_state, $key);
    }
  }

  // Add action buttons.
  if ($storage['current_page'] !== $storage['feedback_page'] && !empty($storage['previous_page']) && !in_array($storage['current_page'], $storage['no_nav_pages']) && count($form_state['storage']['nav_history']) > 2)  {
    $form['action']['previous_page'] = array(
      '#type' => 'submit',
      '#value' => 'Terug',
      '#name' => 'previous_page',
      '#attributes' => array('class' => array('btn hollow previous')),
      '#ajax' => array(
        'wrapper' => 'newssubscription_form_wrapper',
        'callback' => 'newssubscription_ajax_callback',
      ),
    );
  }

  if ($storage['current_page'] !== $storage['final_submit_page']) {
    $next_button_text = 'Verder';
  }
  else {
    $next_button_text = 'Insturen';
  }
  if ($storage['current_page'] !== $storage['feedback_page'] && empty($form['action']['next_page'])) {
    $form['action']['next_page'] = array(
      '#type' => 'submit',
      '#value' => $next_button_text,
      '#name' => 'next_page',
      '#attributes' => array('class' => array('btn','next')),
      '#ajax' => array(
        'wrapper' => 'newssubscription_form_wrapper',
        'callback' => 'newssubscription_ajax_callback',
      ),
    );
  }
  if (!empty($form['action'])) {
    $form['action']['#type'] = 'fieldset';
    $form['action']['#attributes'] = array('class' => array('newssubscription-action'));
  }

// Clear form history when on final page.
if ($storage['current_page'] === $storage['feedback_page']) {
  newssubscription_clear_form($form_state);
}

  return $form;
}

/**
 * Validate function for newssubscription form.
 */
function newssubscription_form_pre_custom_validate($form, &$form_state) {
  $values = &$form_state['values'];
  $storage = &$form_state['storage'];

  // Trim string values.
  foreach ($values as $key => $value) {
    if (!empty($value) && is_string($value)) {
      $values[$key] = trim($value);
    }
  }

  // Implement required fields validation.
  $trigger = $form_state['triggering_element']['#name'];
  preg_match('/^go_to_([a-z_]+)$/', $trigger, $matches);
  if ($storage['current_page'] == end($storage['nav_history']) && ($trigger == 'previous_page' || !empty($matches[1]))) {
    $form_state['validate'] = FALSE;
  }
  if ($form_state['validate']) {
    foreach ($values as $key => $value) {
      if (isset($storage['required_fields'][$key])) {
        if (empty($value)) {
          form_set_error($key, 'Dit moeten we echt weten.');
        }
      }
    }
  }
}

/**
 * Validate function for newssubscriptionform.
 */
function newssubscription_form_post_custom_validate($form, &$form_state) {
  $values = $form_state['values'];
  $storage = &$form_state['storage'];

  // Store values.
  foreach ($values as $key => $value) {
    $storage['values'][$storage['current_page']][$key] = $value;
  }

  // Make sure form data is stored on ajax callback.
  if (empty($storage['sid'])) {
    $sid = db_insert('newssubscription_storage')
    ->fields(array(
      'form_values' => '',
    ))
    ->execute();
    $storage['sid'] = $sid;
  }

  // Store data.
  newssubscription_store_form_data($storage);

  // Store submission id in session.
  $_SESSION['newssubscription'][$storage['form_name']]['sid'] = $storage['sid'];

  $form_state['form_errors'] = form_get_errors();
  newssubscription_remove_errors($form_state);
  unset($form_state['input']);
}

/**
 * Submit function for newssubscriptionform.
 */
function newssubscription_form_submit($form, &$form_state) {

  // Rebuild form when errors occured.
  if (isset($form_state['form_errors'])) {
    $form_state['rebuild'] = TRUE;
    return;
  }

  $values = $form_state['values'];
  $storage = &$form_state['storage'];

  // Store values.
  unset($storage['values'][$storage['current_page']]);
  foreach ($values as $key => $value) {
    if (!empty($value)) {
      $storage['values'][$storage['current_page']][$key] = $value;
    }
  }

  // Get current naviation state.
  $nav = newssubscription_get_nav($storage);

  // Clear navigation when new next page.

  // Process previous, next page.
  $trigger = $form_state['triggering_element']['#name'];

  switch ($trigger) {
    case 'previous_page':
      if (!empty($nav['previous_page'])) {
        $storage['current_page'] = $nav['previous_page'];
      }
      break;

    case 'next_page':
    case 'call_back':
      if ($nav['next_page'] && !in_array($storage['next_page'], $storage['no_nav_pages']) && $storage['next_page'] !== $nav['next_page']) {
        $pos = array_search($storage['current_page'], $storage['nav_history']);
        $storage['nav_history'] = array_splice($storage['nav_history'], 0, $pos + 1);
      }
      $storage['current_page'] = $storage['next_page'];
      break;
  }

  // Process form navigation.
  preg_match('/^go_to_([a-z_]+)$/', $trigger, $matches);
  if (!empty($matches[1])) {
    $storage['current_page'] = $matches[1];
  }

  // Add current page to history.
  if (array_search($storage['current_page'], $storage['nav_history']) === FALSE) {
    $storage['nav_history'][] = $storage['current_page'];
  }

  // Store next and previous pages.
  $nav = newssubscription_get_nav($storage);
  $storage['previous_page'] = $nav['previous_page'];
  $storage['next_page'] = $nav['next_page'];

  // Store form data in database.
  newssubscription_store_form_data($storage);

  // Store submission id in session.
  $_SESSION['newssubscription'][$storage['form_name']]['sid'] = $storage['sid'];

  $form_state['rebuild'] = TRUE;
}

/**
 * Ajax callback function for newssubscription.
 */
function newssubscription_ajax_callback($form, &$form_state) {
  $storage = $form_state['storage'];
  $trigger = $form_state['triggering_element']['#name'];
  $nav = newssubscription_get_nav($storage);
  $commands = array();
  $commands[] = ajax_command_html('#newssubscription_form_wrapper', render($form));
  switch ($trigger) {
    case 'previous_page':
    case 'next_page':
      $title = $storage['pages'][$nav['current_page']];
      $base_path = base_path();
      $settings_callback = 'newssubscription_' . $storage['form_name'] . '_settings';
      $form_path_name = $settings_callback('path_name');
      $page_name = pathauto_cleanstring($title);
      $path = $base_path . $form_path_name . '/' . $page_name;
      $commands[] = array(
        'command' => 'newssubscription_ga',
        'title'  => $title,
        'path'  => $path,
      );
      break;
  }
  return array('#type' => 'ajax', '#commands' => $commands);
}
