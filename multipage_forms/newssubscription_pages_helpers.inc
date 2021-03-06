<?php

/**
 * Splits name in first, middle, and last name part.
 *
 * @param string $name
 *   The name variable.
 *
 * @return array
 *   Split name parts.
 */

function newssubscription_split_name($name) {
  $matches = array();
  $name = trim($name);
  $name = trim($name, '.,');
  // Remove comma from name.
  $comma_pos = strpos($name, ',');
  if ($comma_pos !== FALSE) {
    $first_half = trim(substr($name, 0, $comma_pos));
    $last_half = trim(substr($name, $comma_pos + 1));
    $name = $last_half . ' ' . $first_half;
  }
  // Replace terms.
  $replace = array(
    'v' => 'van',
    'v.' => 'van',
    'vd' => 'van de',
    'vd.' => 'van de',
    'v.d.' => 'van de',
    'v/d' => 'van de',
  );
  foreach ($replace as $orig => $new) {
    if (strpos($name, ' ' . $orig . ' ') !== FALSE) {
      $name = str_replace($orig, $new, $name);
    }
  }
  // Regexes for title and middle names.
  $titles_regex = 'Mr|mr|Mevr|mevr|Dhr|dhr|Ir|ir|Ing|ing|Bc|bc|Ms|ms|Drs|drs|Dr|dr';
  $middle_names_array = newssubscription_middle_names();
  $middle_names_regex = implode('|', $middle_names_array);
  // Split name on middle_name.
  $regex = "@^(?'title'(\b(" . $titles_regex . ")\b\.?\s)?)(?'first_name'(([A-ZÀÈÌÒÙÁÉÍÓÚÝÂÊÎÔÛÃÑÕÄËÏÖÜŸÇÅa-zàèìòùáéíóúýâêîôûãñõäëïöü¡¿çãñõ'\-\.]+)\s*)+?)(?'middle_name'(\b(" . $middle_names_regex . ")\b\s*))(?'last_name'(\b[A-ZÀÈÌÒÙÁÉÍÓÚÝÂÊÎÔÛÃÑÕÄËÏÖÜŸÇÅa-zàèìòùáéíóúýâêîôûãñõäëïöü¡¿çãñõ'][a-zàèìòùáéíóúýâêîôûãñõäëïöü¡¿çãñõ'\-]+\s?)+)$@";
  $result = preg_match($regex, $name, $matches);
  if (!preg_match($regex, $name, $matches)) {
    // Split name on middle name, last name first.
    $regex = "@^(?'title'(\b(" . $titles_regex . ")\b\.?\s)?)(?'last_name'(\b[A-ZÀÈÌÒÙÁÉÍÓÚÝÂÊÎÔÛÃÑÕÄËÏÖÜŸÇÅa-zàèìòùáéíóúýâêîôûãñõäëïöü¡¿çãñõ'][A-ZÀÈÌÒÙÁÉÍÓÚÝÂÊÎÔÛÃÑÕÄËÏÖÜŸÇÅa-zàèìòùáéíóúýâêîôûãñõäëïöü¡¿çãñõ'\-]+\s?)+),?\s(?'first_name'(([A-ZÀÈÌÒÙÁÉÍÓÚÝÂÊÎÔÛÃÑÕÄËÏÖÜŸÇÅa-zàèìòùáéíóúýâêîôûãñõäëïöü¡¿çãñõ'\-\.]+)\s*)+?)(?'middle_name'(\b(" . $middle_names_regex . ")\b\s*))$@";
    $result = preg_match($regex, $name, $matches);
    if (!preg_match($regex, $name, $matches)) {
      // Split name without middle_name in first name and last name(s).
      $regex = "@^(?'title'(\b(" . $titles_regex . ")\b\.?\s)?)(?'first_name'(([A-ZÀÈÌÒÙÁÉÍÓÚÝÂÊÎÔÛÃÑÕÄËÏÖÜŸÇÅa-zàèìòùáéíóúýâêîôûãñõäëïöü¡¿çãñõ'\-\.]+)\s*)+?)(?'last_name'(\b[A-ZÀÈÌÒÙÁÉÍÓÚÝÂÊÎÔÛÃÑÕÄËÏÖÜŸÇÅa-zàèìòùáéíóúýâêîôûãñõäëïöü¡¿çãñõ'][A-ZÀÈÌÒÙÁÉÍÓÚÝÂÊÎÔÛÃÑÕÄËÏÖÜŸÇÅa-zàèìòùáéíóúýâêîôûãñõäëïöü¡¿çãñõ'\-]+\s?)+)$@";
      $result = preg_match($regex, $name, $matches);
      if (!preg_match($regex, $name, $matches)) {
        // Split name in first name and last name, last name first.
        $regex = "@^(?'title'(\b(" . $titles_regex . ")\b\.?\s)?)(?'last_name'(\b[A-ZÀÈÌÒÙÁÉÍÓÚÝÂÊÎÔÛÃÑÕÄËÏÖÜŸÇÅa-zàèìòùáéíóúýâêîôûãñõäëïöü¡¿çãñõ'][A-ZÀÈÌÒÙÁÉÍÓÚÝÂÊÎÔÛÃÑÕÄËÏÖÜŸÇÅa-zàèìòùáéíóúýâêîôûãñõäëïöü¡¿çãñõ'\-]+\s?)+),?\s+(?'first_name'((([A-ZÀÈÌÒÙÁÉÍÓÚÝÂÊÎÔÛÃÑÕÄËÏÖÜŸÇÅa-zàèìòùáéíóúýâêîôûãñõäëïöü¡¿çãñõ']\.)+)\s*)+?)$@";
        $result = preg_match($regex, $name, $matches);
        if (!preg_match($regex, $name, $matches)) {
          // Check if only first name.
          $regex = "@^(?'first_name'([A-ZÀÈÌÒÙÁÉÍÓÚÝÂÊÎÔÛÃÑÕÄËÏÖÜŸÇÅa-zàèìòùáéíóúýâêîôûãñõäëïöü¡¿çãñõ'][a-zàèìòùáéíóúýâêîôûãñõäëïöü¡¿çãñõ'\-]+))$@";
          $result = preg_match($regex, $name, $matches);
          if (!preg_match($regex, $name, $matches)) {
            // No match found.
            return FALSE;
          }
        }
      }
    }
  }
  // Set first character uppercase.
  $parts = array(
    'first_name',
    'last_name',
  );
  foreach ($parts as $part) {
    if (!empty($matches[$part])) {
      $matches[$part] = ucfirst($matches[$part]);
    }
  }
  foreach ($matches as $part => $value) {
    $matches[$part] = trim($value);
  }
  return $matches;
}

/**
 * Return array with middle names.
 *
 * @return array
 *   Middle names.
 */
function newssubscription_middle_names() {
  return array(
    "Uijt te de", "van van de", "uijt te de", "Voor in ‘t", "De van der", "de van der", "Van van de", "voor in ‘t", "Uit te de", "onder den", "Voor in t", "onder het", "van de l’", "De die le", "de die le", "Van de l’", "voor in t", "Onder het", "Onder den", "uit te de", "over het", "uijt den", "Onder de", "boven d’", "uijt ten", "Uijt ten", "over den", "Over den", "onder de", "onder ‘t", "Onder ‘t", "Van de l", "Boven d’", "van de l", "voor den", "Over het", "Uijt den", "Voor den", "Over ‘t", "Boven d", "Von dem", "boven d", "Von der", "von den", "Von den", "von der", "von dem", "Onder t", "van der", "Van ter", "Van gen", "van het", "van gen", "Van het", "van ter", "over ‘t", "van den", "Van den", "Van Der", "Van der", "onder t", "Van Den", "bij den", "auf dem", "Auf den", "Auf dem", "voor de", "uit het", "Uit het", "auf den", "Auf der", "Voor de", "voor ‘t", "auf ter", "Auf ter", "auf der", "aan het", "Aan het", "uijt de", "vor der", "Uijt de", "uijt ‘t", "Uijt ‘t", "Vor der", "Uit ten", "Aan der", "aan der", "aan den", "Aan den", "uit ten", "uit den", "Uit den", "aus der", "Bij het", "Bij den", "Voor ‘t", "over de", "Over de", "bij het", "Aus dem", "aus dem", "Aus den", "Aus der", "aus den", "in het", "die le", "In der", "in der", "In het", "Uit de", "van ‘t", "Van de", "van De", "van de", "Van ‘t", "Uit ‘t", "In den", "uit de", "uit ‘t", "in den", "Op ten", "op het", "Over t", "Op het", "Die le", "op gen", "over t", "op ten", "Von ‘t", "Bij ‘t", "aus ‘m", "bij ‘t", "Bij de", "von ‘t", "bij de", "Op gen", "op der", "Aan ‘t", "aan ‘t", "De las", "de las", "Van la", "van la", "Aan de", "aan de", "op den", "Op der", "De die", "de die", "Op den", "Aus ‘m", "Onder", "op ‘t", "Op ‘t", "Op de", "op de", "onder", "Uit t", "Van t", "De l’", "uit t", "Dalla", "de l’", "De la", "de le", "De le", "de la", "Von t", "bij t", "Am de", "aan t", "Aan t", "am de", "Aus m", "Bij t", "von t", "aus m", "Degli", "dalla", "van t", "In de", "in ‘t", "In ‘t", "della", "in de", "unter", "Unter", "Della", "degli", "deca", "uijt", "Over", "over", "Voor", "in t", "In t", "Uijt", "voor", "op t", "De l", "thoe", "de l", "thor", "Deca", "Thor", "Dal’", "Thoe", "Op t", "dal’", "zum", "Zur", "toe", "zur", "tot", "Toe", "Tot", "uit", "vom", "Von", "Vom", "ver", "Ver", "van", "Van", "vor", "Vor", "Uit", "von", "Zum", "Los", "Den", "den", "del", "Del", "Dei", "dei", "Der", "der", "dos", "Het", "Dos", "don", "Des", "des", "das", "Das", "Aus", "aus", "auf", "Auf", "Aan", "aan", "Ben", "ben", "Dal", "dal", "bin", "Bin", "Bij", "bij", "het", "Don", "Ter", "las", "ter", "ten", "Ten", "los", "les", "Les", "Tho", "Las", "tho", "d’", "‘T", "‘t", "D’", "Da", "da", "l’", "op", "S’", "‘s", "s’", "te", "af", "am", "Am", "al", "Af", "de", "Al", "zu", "Zu", "to", "Te", "De", "du", "El", "Du", "Le", "do", "Op", "el", "la", "in", "L’", "In", "im", "La", "Im", "Do", "To", "lo", "di", "‘S", "Of", "Lo", "le", "of", "Di", "A", "l", "S", "s", "L", "I", "T", "t", "D", "d", "i", "a",
  );
}

/**
 * Returns unique hash.
 */
function newssubscription_get_hash($membership_type, $name) {
  return md5($membership_type . $name . time());
}

/**
 * Creates overview table.
 *
 * @return string
 *   Table markup.
 */
function newssubscription_overview_table($form_state) {
  $storage = $form_state['storage'];
  $values = $storage['values'];
  $settings_callback = 'newssubscription_' . $storage['form_name'] . '_settings';
  $pages = $settings_callback('pages');

  $markup = '';
  $rows = array();

  // Loop through pages.
  $values = array_merge($storage['pages'], $values);
  foreach ($values as $page => $items) {
    if (in_array($page, $storage['nav_history'])) {
      $page_title = (empty($pages[$page])) ? '' : $pages[$page];
      $rows_tmp[] = array(
        'data' => array(
          0 => array(
            'data' => $page_title,
            'header' => TRUE,
            'colspan' => 2,
          ),
        ),
        'no_striping' => TRUE,
      );
      // Loop through form elements.
      if (is_array($items)) {
        foreach ($items as $key => $value) {
          if (!empty($storage['form_element_definitions'][$key])) {
            $form_element = $storage['form_element_definitions'][$key];
            $title = (empty($form_element['#title'])) ? '' : $form_element['#title'];
            if (strpos($key, 'page_') === 0) {
              // Parse output of different form element types.
              switch ($form_element['#type']) {
                case 'textfield':
                case 'emailfield':
                case 'telfield':
                  $rows_tmp[] = array($title, check_plain($value));
                  break;

                case 'numberfield':
                  $rows_tmp[] = array(
                    $title,
                    str_replace('.', ',', newssubscription_clean_amount($value)),
                  );
                  break;

                case 'date_select':
                  $date = date("d-m-Y", strtotime($value));
                  $rows_tmp[] = array($title, $date);
                  break;

                case 'select':
                case 'radios':
                  $display = $form_element['#options'][$value];
                  $rows_tmp[] = array(
                    $title,
                    preg_replace('/<img[^>]+\>/i', '', $display),
                  );
                  break;

                case 'checkboxes':
                  $list = array();
                  foreach ($value as $key => $val) {
                    if ($key === $val) {
                      $list[$key] = check_plain($form_element['#options'][$val]);
                    }
                  }
                  if (count($form_element['#options']) === 1) {
                    if (empty($list)) {
                      $list = array('Nee');
                    }
                    else {
                      $list = array('Ja');
                    }
                  }
                  else {
                    if (empty($list)) {
                      $list = array('-');
                    }
                  }
                  $rows_tmp[] = array(
                    $title,
                    newssubscription_natural_language_join($list, 'en'),
                  );
                  $list = array();
                  break;
              }
            }
          }
        }
      }
      // Do not output pages without interactive elements.
      if (count($rows_tmp) > 1) {
        $rows = array_merge($rows, $rows_tmp);
      }
      $rows_tmp = array();
    }
  }

  // Create table.
  $markup = theme('table', array('rows' => $rows));

  return $markup;
}

/**
 * Helper function to cleanup user input amount.
 */
function newssubscription_clean_amount($amount) {
  $amount = preg_replace('/[^0-9,.]/', '', $amount);
  $amount = str_replace(',', '.', $amount);
  if (!is_numeric($amount)) {
    return FALSE;
  }
  else {
    $amount = number_format($amount, 2, '.', '');
  }
  return $amount;
}

/**
 * Join a string with a natural language conjunction at the end.
 *
 * Https://gist.github.com/angry-dan/e01b8712d6538510dd9c.
 */
function newssubscription_natural_language_join(array $list, $conjunction = 'and') {
  $last = array_pop($list);
  if ($list) {
    return implode(', ', $list) . ' ' . $conjunction . ' ' . $last;
  }
  return $last;
}

function newssubscription_store_submission($form_state) {
  $storage = $form_state['storage'];
  $values = $storage['values'];

  // Set mobile, or phone.
  $fields = array(
    'name' => newssubscription_hv($values['start']['page_start_name']),
    'firstname' => newssubscription_hv($storage['first_name']),
    'middlename' => newssubscription_hv($storage['middle_name']),
    'lastname' => newssubscription_hv($storage['last_name']),
    'email' => newssubscription_hv($values['start']['page_start_email']),
    'contact_id' => NULL,
    'created' => newssubscription_hv($storage['created']),
    'verified' => NULL,
    'reminded' => NULL,
    'submitted' => NULL,
    'frequency' => newssubscription_hv($values['select']['page_select_frequence']),
  );

  // Store newsletter subscription.
  if ($values['select']['page_select_newsletter']['newsletter'] === 'newsletter') {
    $fields['newsletter'] = 1;
  }

  // Store subscribed themes.
  foreach ($values['select']['page_select_themes'] as $key => $val) {
    if ($key === $val) {
      $fields[$key] = 1;
    }
  }

  db_merge('newssubscription_submissions')
    ->key(array(
      'hash' => newssubscription_hv($storage['hash']),
    ))
    ->fields($fields)
    ->execute();
  $submission_id = Database::getConnection()->lastInsertId();

  return array('submission_id' => $submission_id, 'fields' => $fields);
}

/**
 * Helper function.
 */
function newssubscription_hv($value) {
  return empty($value) ? NULL : $value;
}
