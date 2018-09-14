<?php

function newssubscription_test_form($form, &$form_state) {
  $nodes = array(
    node_load(19714),
    node_load(19713),
  );
  $mail_content = newssubscription_get_mail_content($nodes);

  $form['content'] = array(
    '#markup' => $mail_content['content'],
  );

  $form['test'] = array(
    '#type' => 'submit',
    '#value' => 'Send test mail',
  );
  return $form;
}

function newssubscription_test_form_submit($form, &$form_state) {
  $nodes = array(
    node_load(19714),
    node_load(19713),
  );

  $content = newssubscription_get_mail_content($nodes);

  $to = 'tdgraaff@sp.nl';
  $language = language_default();

  $from = variable_get('site_mail', ini_get('sendmail_from'));
  drupal_mail('newssubscription', 'new-content-message', $to, $language, $content, $from, TRUE);
}
