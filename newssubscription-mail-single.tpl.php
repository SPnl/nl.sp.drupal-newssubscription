<?php
  /*
   * Available variables:
   *
   * $title
   * $image
   *  - uri
   *  - name
   * $themes
   *   - [nr]
   *     - title
   *     - path
   * $short_text
   * $sections
   *   - [nr]
   *     - type
   *     - type afbeelding: has also path, caption, text
   *     - type text: has also value
   *     - type quote_short: has also value
   *     - type kader: has also value
   * $persons
   *   - [nr]
   *     - title
   *     - path
   *     - image_path
   *
   */
?>
<h1><?php print $title; ?></h1>
<?php print theme_image_style(array('style_name' => 'actualiteit_item_crop_480_480', 'path' => $image['uri'])); ?>
<?php foreach ($themes as $theme) : ?>
  <?php $theme_titles[] = $theme['title']; ?>
<?php endforeach; ?>
<p><?php print implode(',', $theme_titles); ?></p>
<?php foreach ($sections as $section) : ?>
<?php switch ($section['type']): ?>
<?php case 'quote_short': ?>
<q><?php print $section['value']; ?></q>
<?php break; ?>
<?php case 'text': ?>
<p><?php print $section['value']; ?></p>
<?php break; ?>
<?php case 'afbeelding': ?>
<?php print theme_image_style(array('style_name' => 'thumbnail', 'path' => $section['path'])); ?>
<?php if (!empty($section['text'])): ?>
<p><?php print $section['text']; ?></p>
<?php endif; ?>
<?php break; ?>
<?php case 'kader': ?>
<blockquote><?php print $section['value']; ?></blockquote>
<?php break; ?>
<?php endswitch; ?>
<?php endforeach; ?>
<p><a href="https://www.sp.nl/<?php print $person['path']; ?>">Lees online</a></p>
<?php foreach ($persons as $person) : ?>
  <?php print theme_image_style(array('style_name' => 'actualiteit_item_crop_480_480', 'path' => $person['image_path'])); ?>
<?php endforeach; ?>
