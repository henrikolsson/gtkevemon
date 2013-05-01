<?php
/*
 * This file is part of the GtkEveMon Backend.
 * Written by Simon Fuhrmann
 *
 * GtkEveMon is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

ini_set('display_errors', TRUE);
date_default_timezone_set("GMT");

require_once("phplib/template.php");
require_once("phplib/versionxml.php");

$tpl = new Template("index.tpl", "Versioning Backend");
$tpl->printHeader();

$vxml = new VersionXML("versions.xml");

?>

  <div class="codeblock"><?php $vxml->print_update_xml(); ?></div>

  <h3>Applications</h3>
  <?php $vxml->print_applications(); ?>
  <h3>Data Files</h3>
  <?php $vxml->print_datafiles(); ?>
  <h3>Administration</h3>
  <p>Go to the <a href="admin.php">administration page</a>.</p>

  <div style="clear: both;"></div>

<?php

$tpl->printFooter();
