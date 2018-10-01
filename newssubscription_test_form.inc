<?php

function newssubscription_test_form($form, &$form_state) {
  $types = nu_get_types();
  $typestring = implode("','", $types);
  $query = "SELECT nid FROM {node} WHERE status = 1 AND type IN ('" . $typestring . "') ORDER BY nid DESC LIMIT 5";
  $node_ids = db_query($query)->fetchCol();
  $nodes = node_load_multiple($node_ids);
  $form_state['nodes'] = $nodes;
  $mail_content = newssubscription_get_mail_content($nodes, 'weekly_digest', 'Thomas de Graaff');

  $form['content'] = array(
    '#markup' => $mail_content['content'],
  );

  $form['mail'] = array(
    '#type' => 'textfield',
    '#title' => 'E-mailadres',
    '#default_value' => variable_get('newssubscription_test_mail'),
  );

  $types = array(
    'weekly_digest' => 'Wekelijks overzicht',
    'daily_digest' => 'Dagelijks overzicht',
    'direct_mail' => 'Direct',
  );

  $form['type'] = array(
    '#type' => 'radios',
    '#options' => $types,
    '#default_value' => 'weekly_digest',
  );

  $form['test'] = array(
    '#type' => 'submit',
    '#value' => 'Send test mail',
  );
  return $form;
}

function newssubscription_test_form_submit($form, &$form_state) {
  $nodes = $form_state['nodes'];
  shuffle($nodes);
  if ($form_state['values']['type'] === 'direct_mail') {
    $single_nodes[] = array_shift($nodes);
    $nodes = $single_nodes;
  }

  variable_set('newssubscription_test_mail', $form_state['values']['mail']);

  $content = newssubscription_get_mail_content($nodes, $form_state['values']['type'], 'Thomas de Graaff');
  $to = $form_state['values']['mail'];
  $language = language_default();
  $from = variable_get('site_mail', ini_get('sendmail_from'));

  $params = array(
    'subject' => 'TEST: ' . $content['subject'],
    'content' => $content['content'],
    'name' => '',
    'encoded_id' => newssubscription_encode(761666),
  );
  $result = drupal_mail('newssubscription', 'new-content-message', $to, $language, $params, $from, TRUE);
}
