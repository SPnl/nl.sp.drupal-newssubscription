<?php
  /*
   * Available variables:
   *
   * $items
   *   - [nr]
   *     title
   *     image
   *     - uri
   *     - name
   *     themes
   *      - [nr]
   *        - title
   *        - path
   *     short_text
   *     sections
   *      - [nr]
   *        - type
   *        - type afbeelding: has also path, caption, text
   *        - type text: has also value
   *        - type quote_short: has also value
   *        - type kader: has also value
   *     persons
   *      - [nr]
   *        - title
   *        - path
   *        - image_path
   *
   */
?>
<?php foreach ($items as $item): ?>
<h1><?php print $item['title']; ?></h1>
<?php if (!empty($item['image']['uri'])) : ?>
<?php print theme_image_style(array('style_name' => 'actualiteit_item_crop_480_480', 'path' => $item['image']['uri'], 'width' => 200, 'height' => 200)); ?>
<?php endif; ?>
<?php $theme_titles = array(); ?>
<?php foreach ($item['themes'] as $theme) : ?>
  <?php $theme_titles[] = $theme['title']; ?>
<?php endforeach; ?>
<p><?php print implode(',', $theme_titles); ?></p>
<?php print $item['short_text']; ?>
<p><a href="https://www.sp.nl/<?php print $item['path']; ?>">Lees verder</a></p>
<?php foreach ($item['persons'] as $person) : ?>
<?php if (!empty($person['image_path'])) : ?>
<a href="https://www.sp.nl/<?php print $person['path']; ?>"><?php print theme_image_style(array('style_name' => 'actualiteit_item_crop_480_480', 'path' => $person['image_path'], 'width' => 200, 'height' => 200)); ?></a>
<?php endif; ?>
<?php endforeach; ?>
<?php endforeach; ?>
