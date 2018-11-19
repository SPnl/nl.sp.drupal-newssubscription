<?php

function newssubscription_send_news() {
  $batch_size = 200;
  // Weekly digest (sunday = 0, monday = 1 etc.).
  if (date('w') == 6 && variable_get('newssubscription_weekly_digest_last_cron', 0) < time() - 86400) {
    newssubscription_send_news_by_type('weekly_digest', $batch_size);
  }
  // Daily digest (hour = 0-23).
  if (date('G') == 16 && variable_get('newssubscription_daily_digest_last_cron', 0) < time() - 3600) {
    newssubscription_send_news_by_type('daily_digest', $batch_size);
  }
  // Direct mail.
  newssubscription_send_news_by_type('direct_mail', $batch_size);
}

function newssubscription_send_news_by_type($type, $batch_size) {
  variable_set('newssubscription_' . $type . '_last_cron', time())  ;
  // Get news nodes.
  $nodes = newssubscription_get_news_nodes($type);
  if ($nodes) {
    watchdog('newssubscription', 'Sending ' . $type . ' news.');
    watchdog('newssubscription', 'Found nodes:' . print_r($nodes, TRUE));
    // Get recipients (in batches).
    $batches = newssubscription_get_number_of_batches($type, $batch_size);
    if ($batches) {
      $recipients = array();
      for ($batch = 0; $batch < $batches; $batch++) {
        $recipients = array();
        newssubscription_get_news_recipients($type, $batch_size, $batch, $recipients);
        if ($recipients) {
          watchdog('newssubscription', 'Found recipients:' . print_r($recipients, TRUE));
          // Create send data..
          foreach ($recipients as $recipient) {
            $recipient_nodes = array();
            if (!empty($recipient['themes'])) {
              foreach ($recipient['themes'] as $theme) {
                if (!empty($nodes[$theme])) {
                  $recipient_nodes = array_merge($recipient_nodes, $nodes[$theme]);
                }
              }
              if (!empty($recipient_nodes)) {
                $news_to_send_data[$recipient['email']] = array(
                  'node_ids' => $recipient_nodes,
                  'first_name' => empty($recipient['firstname']) ? '' : $recipient['first_name'],
                  'display_name' => $recipient['display_name'],
                  'email' => $recipient['email'],
                  'contact_id' => $recipient['contact_id'],
                  'type' => $type,
                );
              }
            }
          }
          if (!empty($news_to_send_data)) {
            watchdog('newssubscription', 'Found news to send:' . print_r($news_to_send_data, TRUE));
            newssubscription_send_news_mail($news_to_send_data);
          }
        }
      }
    }
  }
}

function newssubscription_send_news_mail($news_to_send) {
  if ($news_to_send) {
    $delay = 0;
    $max_messages_per_second = variable_get('newssubscription_max_messages_per_second', 10);
    $min_time_per_message = ((1 / $max_messages_per_second) / 1000000);
    watchdog('test', 'Min time per message: ' . $min_time_per_message);

    array_reverse($news_to_send);
    foreach ($news_to_send as $news_message_data) {
      usleep($delay);

      $begin_time = microtime(TRUE);
      watchdog('test', 'Begin time: ' . $begin_time);
      $nodes = node_load_multiple($news_message_data['node_ids']);
      $name = empty($news_message_data['first_name']) ? $news_message_data['display_name'] : $news_message_data['first_name'];
      $type = $news_message_data['type'];
      $content = newssubscription_get_mail_content($nodes, $type, $name);
      $mail_data = array(
        'subject' => $content['subject'],
        'content' => $content['content'],
        'name' => $name,
        'encoded_id' => newssubscription_encode($news_message_data['contact_id']),
      );

      $to = $news_message_data['email'];
      $language = language_default();

      $from = variable_get('site_mail', ini_get('sendmail_from'));
      $result = drupal_mail('newssubscription', 'new-content-message', $to, $language, $mail_data, $from, TRUE);
      watchdog('newssubscription', 'Result drupal mail:' . print_r($result, TRUE));
      $end_time = microtime(TRUE);
      watchdog('test', 'End time: ' . $end_time);

      $delay_new = $min_time_per_message - $end_time + $begin_time;
      watchdog('test', 'Delay new: ' . $delay_new);

      if ($delay_new > 0) {
        $delay = $delay_new;
      }
      else {
        $delay = 0;
      }
      watchdog('test', 'Delay: ' . $delay);
    }
  }
}

function newssubscription_get_news_nodes($type) {
  $news_nodes = array();
  $news_types = nu_get_types();
  $type_filter = "'" . implode("','", $news_types) . "'";

  $theme_nids = newssubscription_get_themes_nids();

  $since_date = max(variable_get('newssubscription_' . $type . '_last_creation_date'), variable_get('newssubscription_active_date', time()));

  $last_created = 0;

  foreach ($theme_nids as $theme => $theme_nid) {
    // TODO: filter publication nodes that have parent node.
    $query = "SELECT nid, created FROM field_data_thema t INNER JOIN node n ON t.entity_id = n.nid WHERE status = 1 AND type IN (" . $type_filter . ") AND thema_nid = :theme_nid AND created > :since_date";
    $result = db_query($query, array(
      ':theme_nid' => $theme_nid,
      ':since_date' => $since_date,
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
      'return' => "custom_" . $newssubscription_themes_field . ",display_name,email,first_name,id",
      'custom_' . $newssubscription_frequency_field => array($type_translate[$type]),
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
      if ($result['values']) {
        foreach ($result['values'] as $data) {
          if ($data['custom_' . $newssubscription_themes_field]) {
            $recipients[$data['email']] = array(
              'email' => $data['email'],
              'first_name' => $data['first_name'],
              'display_name' => $data['display_name'],
              'contact_id' => $data['id'],
              'themes' => $data['custom_' . $newssubscription_themes_field],
            );
          }
        }
      }
    }
  }
}

function newssubscription_get_mail_content($nodes, $type, $name) {
  $mail_content = array();

  $typenames = array(
    'daily_digest' => 'dagelijks overzicht ',
    'weekly_digest' => 'wekelijks overzicht ',
    'direct_mail' => '',
  );

  if (count($nodes) === 1) {
    // Create content for single article mail.
    $node = current($nodes);
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
    $subject = ucfirst($typenames[$type]) . 'SP nieuws: "' . check_plain($node->title) . '"';
    //$variables = newssubscription_get_single_mail_data($node);
    $variables = newssubscription_get_digest_mail_data($nodes, $type, $name);
    //$html = theme('newssubscription_theme_mail_single', $variables);
    $html = theme('newssubscription_theme_mail_digest', $variables);
    $mail_content = array(
      'subject' => $subject,
      'content' => $html,
    );
  }
  elseif (count($nodes) > 1) {
    // Create content for digest mail.
    $variables = newssubscription_get_digest_mail_data($nodes, $type, $name);
    $html = theme('newssubscription_theme_mail_digest', $variables);
    $mail_content = array(
      'subject' => ucfirst($typenames[$type]) . 'SP nieuws',
      'content' => $html,
    );
  }
  return $mail_content;
}

function newssubscription_get_single_mail_data($node, $type, $name) {
  $data = array();

  // Get afbeelding.
  $image = array();
  $field_items = field_get_items('node', $node, 'image');
  if (!empty($field_items[0]['uri'])) {
    $image = array(
      'uri' => $field_items[0]['uri'],
      'name' => check_plain($field_items[0]['filename']),
    );
  }

  // Get themes data.
  $themes = array();
  $field_items = field_get_items('node', $node, 'thema');
  if (!empty($field_items)) {
    foreach ($field_items as $field_item) {
      $theme_node = node_load($field_item['nid']);
      $themes[] = array(
        'title' => check_plain($theme_node->title),
        'path' => drupal_get_path_alias('node/' . $theme_node->nid),
      );
    }
  }

  // Get page sections data.
  $sections = array();
  $short_text = '';
  if (!empty($node->actualiteit_content['und'])) {
    $page_section_ids = array();
    foreach ($node->actualiteit_content['und'] as $field_item) {
      $page_section_ids[] = $field_item['sid'];
    }
    if (!empty($page_section_ids)) {
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
            if (!empty($raw_section->image['und'][0]['uri'])) {
              $sections[] = array(
                'type' => 'afbeelding',
                'path' => $raw_section->image['und'][0]['uri'],
                'caption' => empty($raw_section->snippet['und']) ? '' : $raw_section->snippet['und'][0]['safe_value'],
                'text' => empty($raw_section->body['und']) ? '' : $raw_section->body['und'][0]['safe_value'],
              );
            }
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
          /*
          $sections[] = array(
            'type' => 'kader',
            'value' => $raw_section->body['und'][0]['safe_value'],
          );
          if (empty($short_text)) {
            $short_text = $raw_section->body['und'][0]['safe_value'];
          }
          */
          break;

          case 'jaartal':
          break;

          case 'video':
          // Video functionaliteit wordt niet meer gebruikt door communicatie.
          break;
        }
      }
    }
  }

  // Persons.
  $persons = array();
  $field_items = field_get_items('node', $node, 'persoon');
  if (!empty($field_items)) {
    foreach ($field_items as $field_item) {
      $person_node = node_load($field_item['nid']);
      $persons[] = array(
        'title' => check_plain($person_node->title),
        'path' => drupal_get_path_alias('node/' . $person_node->nid),
        'image_path' => !empty($person_node->image['und'][0]['uri']) ? $person_node->image['und'][0]['uri'] : '',
      );
    }
  }

  // Shorten short_text.
  if (!empty($short_text)) {
    $max_length = 200;
    $first_paragraph_position = max(strpos($short_text, '</p>'), strpos($short_text, '</P>'));
    if ($first_paragraph_position === FALSE) {
      $short_text = substr(drupal_html_to_text($short_text), 0, $max_length);
    }
    else {
      $short_text = substr(drupal_html_to_text(substr($short_text, 0, $first_paragraph_position + 1)), 0, $max_length);
    }

    // Find last line.
    $end = max(strrpos($short_text, '.'), strrpos($short_text, '?'), strrpos($short_text, '!'));
    if ($end !== FALSE && $end > ($max_length / 2)) {
      $short_text = substr($short_text, 0, $end + 1);
    }
    else {
      $end = strrpos($short_text, ' ');
      if ($end !== FALSE) {
        $short_text = substr($short_text, 0, $end + 1) . ' ...';
      }
      else {
        $short_text = substr($short_text, 0, $max_length);
      }
    }
  }

  // Create data array for template.
  $data = array(
    'title' => check_plain($node->title),
    'path' => drupal_get_path_alias('node/' . $node->nid),
    'image' => $image,
    'themes' => $themes,
    'short_text' => $short_text,
    'sections' => $sections,
    'persons' => $persons,
    'type' => $type,
    'name' => $name,
  );
  return $data;
}

function newssubscription_get_digest_mail_data($nodes, $type, $name) {
  $data = array();
  foreach ($nodes as $node) {
    $data['items'][] = newssubscription_get_single_mail_data($node, $type, $name);
  }
  $data['type'] = $type;
  $data['name'] = $name;
  return $data;
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

function newssubscription_encode($data, $serialize = FALSE) {
  if ($serialize) {
    $data = serialize($data);
  }
  $key = variable_get('newssubscription_secret', 'paijeich6zoh4Mah');
  $encrypted = openssl_encrypt($data, 'AES-128-ECB', $key);
  $hash = base64_encode($encrypted);
  return $hash;
}

function newssubscription_decode($hash, $unserialize = FALSE) {
  $decoded = base64_decode($hash);
  $key = variable_get('newssubscription_secret', 'paijeich6zoh4Mah');
  $data = openssl_decrypt($decoded, 'AES-128-ECB', $key);
  if ($unserialize) {
    $data = unserialize($data);
  }
  return $data;
}

function newssubscription_unsubscribe($hash) {
  $contact_id = newssubscription_decode($hash);
  if (ctype_digit(strval($contact_id))) {
    $newsletter_field = variable_get('newssubscription_newsletter_field');
    $themes_field = variable_get('newssubscription_themes_field');
    $frequency_field = variable_get('newssubscription_frequency_field');

    $spcivi = \SPCivi::getInstance();

    // Unsubscribe.
    $params = array(
      'sequential' => 1,
      'contact_type' => 'Individual',
      'id' => $contact_id,
      'custom_229' => '',
      'custom_227' => '',
      'custom_228' => '',
    );
    $result = $spcivi->api('Contact', 'create', $params);
    if (!newssubscription_process_civi_result($result, 'Fout bij het verwijderen van een nieuwsaanmelding in CiviCRM.') === FALSE) {
      return '<p>Uw nieuwsaanmelding is verwijderd.</p>';
    }
  }
  drupal_set_message('Er is iets misgegaan met het aanpassen van uw nieuwsaanmeling in ons systeem', 'error');
  return '<p>Probeert u het op een later moment nog eens.</p>';
}
