<?php
  /*
   * Available variables:
   *
   * $type
   * $name
   * $items
   *   - [nr]
   *     type
   *     name
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
<?php $typenames = array('daily_digest' => 'Dagelijks overzicht', 'weekly_digest' => 'Wekelijks overzicht', 'direct_mail' => ''); ?>
<h1 color="black" style="color: black"><?php print $typenames[$type]; ?> SP nieuws</h1>
<?php foreach ($items as $item): ?>
    <table width="600" style="width: 600px">
      <tr>
        <td width="180" height="180" style="width: 180px; height: 180px; padding: 0">
          <?php if (!empty($item['image']['uri'])) : ?>
            <a  style="text-decoration: none" href="<?php global $base_url; print $base_url . '/' . $item['path']; ?>">
              <img src="<?php print image_style_url('mail_crop_180_180', $item['image']['uri']); ?>" />
            </a>
          <?php endif; ?>
        </td>
        <td width="10" style="width: 10px; padding: 0"></td>
        <td valign="top" style="padding: 0">
          <table width="400" height="180" style="width: 400px; height: 180px">
            <tr height="10" style="height: 10px">
              <td align="left" valign="top" style="vertical-align: top; padding: 0">
                <a  style="text-decoration: none" href="<?php global $base_url; print $base_url . '/' . $item['path']; ?>">
                  <h1 style="margin: 0"><?php print $item['title']; ?></h1>
                </a>
              </td>
            </tr>
            <tr>
              <td align="left" valign="top" style="vertical-align: top; padding: 0">
                <p style="margin: 0"><?php print $item['short_text']; ?></p>
              </td>
            </tr>
          </table>
        </td>
      </tr>
    </table>
    <table heigth="13" style="height: 13px"><tr><td></td></tr></table>
<?php endforeach; ?>
