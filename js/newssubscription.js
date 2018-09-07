(function($, Drupal) {
  Drupal.ajax.prototype.commands.newssubscription_ga = function(ajax, response, status) {
    if(window.ga && ga.create) {
      ga('set', { page: response.path, title: response.title });
      ga('send', 'pageview');
    }
  };
  Drupal.behaviors.newssubscriptionModule = {
    attach: function (context, settings) {
      // Code to be run on page load, and
      // on ajax load added here

      // Determine scroll
      var scrollPos = $('.newssubscription-navigation').first().offset().top;

      if(( $( ".messages.error" ).first().length)) {
        scrollPos = $('.messages.error').first().parent().offset().top;
      } else {
        if($( ".newssubscription-navigation" ).length) {
          scrollPos = $('.newssubscription-navigation').offset().top;
        }
      }
      $('html, body').animate({ scrollTop: scrollPos },300);
    }
  };
})(jQuery, Drupal);
