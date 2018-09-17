<?php

function newssubscription_send_news_mail($news_to_send) {
  if ($news_to_send) {
    foreach ($news_to_send as $news_message_data) {
      $nodes = node_load_multiple($news_message_data['node_ids']);
      $content = newssubscription_get_mail_content($nodes);

      $to = $news_message_data['display_name'] . ' <' . $news_message_data['email'] . '>';
      $language = language_default();

      $from = variable_get('site_mail', ini_get('sendmail_from'));
      drupal_mail('newssubscription', 'new-content-message', $to, $language, $content, $from, TRUE);
    }
  }
}

function newssubscription_send_news() {
  $batch_size = 1000;
  // Weekly digest (sunday = 0, monday = 1 etc.).
  if (date('w') === 6 && variable_get('newssubscription_weekly_digest_last_cron') < time() - 86400) {
    newssubscription_send_news_by_type('weekly_digest', $batch_size);
  }
  // Daily digest (hour = 0-23).
  if (date('G') === 16 && variable_get('newssubscription_daily_digest_last_cron') < time() - 3600) {
    newssubscription_send_news_by_type('daily_digest', $batch_size);
  }
  // Direct mail.
  newssubscription_send_news_by_type('direct_mail', $batch_size);
}

function newssubscription_send_news_by_type($type, $batch_size) {
  // Get news nodes.
  $nodes = newssubscription_get_news_nodes($type);
  if ($nodes) {
    // Get recipients (in batches).
    $batches = newssubscription_get_number_of_batches($type, $batch_size);
    if ($batches) {
      $recipients = array();
      $themes = array_keys(newssubscription_get_themes_list());
      for ($batch = 0; $batch < $batches; $batch++) {
        $recipients = array();
        newssubscription_get_news_recipients($type, $batch_size, $batch, $recipients);
        if ($recipients) {
          // Create send data..
          foreach ($themes as $theme) {
            foreach ($recipients[$theme] as $recipient) {
              if (!empty($nodes[$theme])) {
                $news_to_send_data[$recipient['email']] = array(
                  'node_ids' => $nodes[$theme],
                  'first_name' => $recipient['firstname'],
                  'display_name' => $recipient['display_name'],
                  'email' => $recipient['email'],
                );
              }
            }
          }
          if ($news_to_send_data) {
            newssubscription_send_news_mail($news_to_send_data);
          }
        }
      }
    }
  }
}

function newssubscription_get_news_nodes($type) {
  $news_nodes = array();
  $news_types = nu_get_types();
  $type_filter = "'" . implode("','", $news_types) . "'";

  $theme_nids = newssubscription_get_themes_nids();

  $last_send_message_creation_date = variable_get('newssubscription_' . $type . '_last_creation_date', variable_get('newssubscription_active_date', time()));
  $last_created = $last_send_message_creation_date;

  foreach ($theme_nids as $theme => $theme_nid) {
    // TODO: filter publication nodes that have parent node.
    $query = "SELECT nid, created FROM field_data_thema t INNER JOIN node n ON t.entity_id = n.nid WHERE type IN (" . $type_filter . ") AND thema_nid = :theme_nid AND created > :last_send_message_creation_date";
    $result = db_query($query, array(
      ':theme_nid' => $theme_nid,
      ':last_send_message_creation_date' => $last_send_message_creation_date,
    ));
    if ($result) {
      while ($row = $result->fetchAssoc()) {
        $news_nodes[$theme][] = $row['nid'];
        if ($row['created'] > $last_created) {
          $last_created = $row['created'];
        }
      }
    }
  }

  if (!empty($last_created)) {
    variable_set('newssubscription_' . $type . '_last_creation_date', $last_created);
  }
  return $news_nodes;
}

function newssubscription_get_number_of_batches($type, $batch_size) {
  $newssubscription_frequency_field = variable_get('newssubscription_frequency_field');
  $batches = 0;

  if (!empty($newssubscription_frequency_field)) {
    $spcivi = \SPCivi::getInstance();
    $type_translate = array(
      'weekly_digest' => 'wekelijks',
      'daily_digest' => 'dagelijks',
      'direct_mail' => 'direct',
    );

    // Get total number of contacts for subscription type.
    $query = array(
      'sequential' => 1,
      'custom_' . $newssubscription_frequency_field => $type_translate[$type],
      'is_deceased' => 0,
      'is_opt_out' => 0,
      'is_deleted' => 0,
    );
    $result = $spcivi->api('Contact', 'getcount', $query);
    if (newssubscription_process_civi_result($result, 'Fout bij het ophalen van totaal aantal ontvangers (type: ' . $type_translate[$type] . ') uit CiviCRM.') === FALSE) {
      return FALSE;
    }
    $total_recipients = $result['result'];

    if ($total_recipients) {
      // Get news recipients by batch.
      $batches = ceil($total_recipients / $batch_size);
    }
  }
  return $batches;
}

function newssubscription_get_news_recipients($type, $batch_size, $batch, &$recipients) {
  $newssubscription_frequency_field = variable_get('newssubscription_frequency_field');
  $newssubscription_themes_field = variable_get('newssubscription_themes_field');
  if (!empty($newssubscription_frequency_field) && !empty($newssubscription_themes_field)) {
    $spcivi = \SPCivi::getInstance();
    $type_translate = array(
      'weekly_digest' => 'wekelijks',
      'daily_digest' => 'dagelijks',
      'direct_mail' => 'direct',
    );

    $query = array(
      'sequential' => 1,
      'return' => "custom_" . $newssubscription_themes_field . ",display_name,email,first_name",
      'custom_' $newssubscription_frequency_field => array($type_translate[$type]),
      'is_deceased' => 0,
      'is_opt_out' => 0,
      'is_deleted' => 0,
      'options' => array(
        'limit' => $batch_size,
        'offset' => $batch * $batch_size
      ),
    );
    $result = $spcivi->api('Contact', 'get', $query);
    if (newssubscription_process_civi_result($result, 'Fout bij het ophalen van een batch ontvangers (type: ' . $type_translate[$type] . ') uit CiviCRM.')) {
      if ($result[values]) {
        foreach ($result['values'] as $data) {
          if ($data['custom_' . $newssubscription_themes_field]) {
            foreach ($data['custom_' $newssubscription_themes_field] as $theme) {
              $recipients[$theme][] = array(
                'email' => $data['email'],
                'first_name' => $data['first_name'],
                'display_name' => $data['display_name'],
              );
            }
          }
        }
      }
    }
  }
}

function newssubscription_get_mail_content($nodes) {
  $mail_content = array();

  if (count($nodes) === 1) {
    // Create content for single article mail.
    $node = array_pop($nodes);
    $types = nu_get_types();
    $names = array_combine($types, array(
      'nieuwsbericht',
      'opiniestuk',
      'column',
      'rapport',
      'publicatie',
      'video',
      'achtergrond',
    ));
    $subject = 'SP ' . ucfirst($names[$node->type]) . ': ' . check_plain($node->title);
    $variables = newssubscription_get_single_mail_data($node);
    $html = theme('newssubscription_theme_mail_single', $variables);
    $mail_content = array(
      'subject' => $subject,
      'content' => $html,
    );
  }
  elseif (count($nodes) > 1) {
    // Create content for digest  mail.
    $variables = newssubscription_get_digest_mail_data($nodes);
    $html = theme('newssubscription_theme_mail_digest', $variables);
    $mail_content = array(
      'subject' => 'SP nieuws update',
      'content' => $html,
    );
  }
  return $mail_content;
}

function newssubscription_get_single_mail_data($node) {
  $data = array();

  // Get afbeelding.
  $image = array();
  $field_items = field_get_items('node', $node, 'image');
  if (!empty($field_items[0])) {
    $image = array(
      'uri' => $field_items[0]['uri'],
      'name' => check_plain($field_items[0]['filename']),
    );
  }

  // Get themes data.
  $themes = array();
  $field_items = field_get_items('node', $node, 'thema');
  foreach ($field_items as $field_item) {
    $theme_node = node_load($field_item['nid']);
    $themes[] = array(
      'title' => check_plain($theme_node->title),
      'path' => drupal_get_path_alias('node/' . $theme_node->nid),
    );
  }

  // Get page sections data.
  $sections = array();
  $short_text = '';
  foreach ($node->actualiteit_content['und'] as $field_item) {
    $page_section_ids[] = $field_item['sid'];
  }
  $raw_sections = entity_load('page_section', $page_section_ids);
  foreach ($raw_sections as $raw_section) {
    switch ($raw_section->type) {
      case 'quote':
        break;

      case 'quote_short':
        $sections[] = array(
          'type' => 'quote_short',
          'value' => $raw_section->quote['und'][0]['safe_value'],
        );
        break;

      case 'verhaal':
        break;

      case 'text':
        $sections[] = array(
          'type' => 'text',
          'value' => $raw_section->body['und'][0]['safe_value'],
        );
        if (empty($short_text)) {
          $short_text = $raw_section->body['und'][0]['safe_value'];
        }
        break;

      case 'afbeelding':
        if (in_array($raw_section->image_mode['und'][0]['value'], array(1, 5))) {
          $sections[] = array(
            'type' => 'afbeelding',
            'path' => $raw_section->image['und'][0]['uri'],
            'caption' => $raw_section->snippet['und'][0]['safe_value'],
            'text' => $raw_section->body['und'][0]['safe_value'],
          );
        }
        if (!empty($raw_section->body)) {
          $sections[] = array(
            'type' => 'text',
            'value' => $raw_section->body['und'][0]['safe_value'],
          );
          if (empty($short_text)) {
            $short_text = $raw_section->body['und'][0]['safe_value'];
          }
        }
        break;

      case 'kader':
        $sections[] = array(
          'type' => 'kader',
          'value' => $raw_section->body['und'][0]['safe_value'],
        );
        if (empty($short_text)) {
          $short_text = $raw_section->body['und'][0]['safe_value'];
        }
        break;

      case 'jaartal':
        break;

      case 'video':
        // Video functionaliteit wordt niet meer gebruikt door communicatie.
        break;
    }
  }

  // Persons.
  $persons = array();
  $field_items = field_get_items('node', $node, 'persoon');
  foreach ($field_items as $field_item) {
    $person_node = node_load($field_item['nid']);
    $persons[] = array(
      'title' => check_plain($person_node->title),
      'path' => drupal_get_path_alias('node/' . $person_node->nid),
      'image_path' => !empty($person_node->image) ? $person_node->image['und'][0]['uri'] : '',
    );
  }

  // Shorten short_text to first or first two paragraphs.
  $end = strpos($short_text, '</p>');
  if ($end < 100) {
    $end = strpos($short_text, '</p>', $end);
  }
  $short_text = substr($short_text, 0, $end + 4);

  // Create data array for template.
  $data = array(
    'title' => check_plain($node->title),
    'path' => drupal_get_path_alias('node/' . $node->nid),
    'image' => $image,
    'themes' => $themes,
    'short_text' => $short_text,
    'sections' => $sections,
    'persons' => $persons,
  );
  return $data;
}

function newssubscription_get_digest_mail_data($nodes) {
  $data = array();
  foreach ($nodes as $node) {
    $data['items'][] = newssubscription_get_single_mail_data($node);
  }
  return $data;
}

/**
 * Defines e-mail.
 */
function newssubscription_get_new_content_mail(&$message, $params) {
  $message['subject'] = $params['subject'];
  $message['body'][] = $params['content'];
}

function newssubscription_send_reminders() {
  $unverified = db_select('newssubscription_submissions', 's')
    ->fields('s')
    ->condition('created', time() - 86400, '<')
    ->isNull('verified')
    ->isNull('reminded')
    ->execute();

  if ($unverified) {

    while ($record = $unverified->fetchAssoc()) {

      $mail = drupal_mail(
        'newssubscription', 'reminder', $record['email'], LANGUAGE_NONE,
        [
          'first_name'      => $record['firstname'],
          'hash'            => $record['hash'],
          'format'          => SWIFTMAILER_FORMAT_HTML,
        ]
      );

      if (!empty($mail['send'])) {

        db_update('newssubscription_submissions')
          ->fields([
            'reminded' => time(),
          ])
          ->condition('id', $record['id'])
          ->execute();
      }
    }
  }
}
