<?php

/**
 * Action pages.
 */

/**
 * Error page content.
 */
function newssubscription_page_error() {
  return '<div class="newssubscription content"><h1>Aanmelding niet verwerkt</h1><p>Excuus, maar het is
    niet gelukt om je aanmelding goed te verwerken. Probeer het later nog eens, of neem even contact met ons
    op.</p></div>';
}

/**
 * Validation function for newssubscription.
 */
function newssubscription_subscription_validate($hash) {

  $q = db_select('newssubscription_submissions', 's')
    ->fields('s')
    ->condition('hash', $hash, '=')
    ->execute();
  if ($q) {
    $record = $q->fetchAssoc();

    if (empty($record['verified'])) {
      $exec = db_update('newssubscription_submissions')
        ->fields(array(
          'verified' => time(),
        ))
        ->condition('hash', $hash, '=')
        ->execute();

      if ($exec) {
        $output = '<div class="newssubscription content"><h1>Aanmelding bevestigd</h1><p>Bedankt voor het bevestigen van je gegevens.</p>';
        return $output;
      }
    }
    else {
      $output = '<div class="newssubscription content"><h1>Aanmelding reeds bevestigd</h1><p>Bedankt, je aanmelding was reeds bevestigd.</p></div>';
      return $output;
    }
  }

  return '<div class="newssubscription content"><h1>Aanmelding niet bevestigd</h1><p>Er is een fout
    opgetreden bij het verwerken van je aanvraag. Controleer of de link die je gevolgd hebt klopt, en kopieer
    deze eventueel handmatig uit de e-mail.</p></div>';
}
