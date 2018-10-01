<?php if (!empty($image)): ?>
  <figure class="home-image header-image">

    <div class="image-wrapper">
      <div class="image-caption rechtvaardigheid">
        <p>
          <span>Meld je aan voor SP nieuws</span>
        </p>
      </div>
      <?php print render($image); ?>
      <?php $social_url = check_plain(urlencode(url(request_path(), array('absolute' => TRUE)))); ?>
      <ul class="social-links">
        <li class="twitter">
          <a href="https://twitter.com/SPnl" target="_blank"><span>Volg ons op Twitter</span></a>
        </li>
        <li class="facebook">
          <a href="https://www.facebook.com/SocialistischePartij" target="_blank"><span>Volg ons op Facebook</span></a>
        </li>
        <li class="whatsapp">
         <a href="https://api.whatsapp.com/send?phone=31703183044"><span>Whatsapp: 070-3183044</span></a>
        </li>
      </ul>
    </div>
    <?php if (!empty($label)): ?>
      <div class="image-overlay home-image-overlay" style="display: none;"><?php print check_plain($label); ?></div>

    <?php endif; ?>
  </figure>
<?php endif; ?>

<section class="content-wrap">
  <h1>Ontvang SP nieuws</h1>
  <?php print drupal_render($form); ?>
</section>
