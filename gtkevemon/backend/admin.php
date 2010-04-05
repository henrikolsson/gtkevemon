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

require_once("phplib/template.php");
require_once("phplib/versionxml.php");
require_once("phplib/updater.php");
require_once("phplib/userauth.php");

ini_set('display_errors', TRUE);

$auth = new UserAuth();

$tpl = new Template("index.tpl", "Versioning Backend");
$tpl->printHeader();

$vxml = new VersionXML("versions.xml");
$upd = new Updater($vxml);
//$vxml->add_datafile("SkillTree.xml", "4", "Quantum Rise update");
//$vxml->add_application("GtkEveMon", "1.5-74", "GtkEveMon update!");
//$vxml->save_versionfile();

?>

  <div class="codeblock"><?php $vxml->print_update_xml(); ?></div>

  <h3>Applications</h3>
  <?php $vxml->print_applications(); ?>
  <h3>Data Files</h3>
  <?php $vxml->print_datafiles(); ?>
  <h3>Administration</h3>

  <table>
    <tr><td><img src="media/icon-add.png"/></td><td><a
    href="admin.php?upd=app">Add / update application</a></td></tr>
    <tr><td><img src="media/icon-add.png"/></td><td><a
    href="admin.php?upd=data">Add / update data file</a></td></tr>
    <tr><td><img src="media/icon-edit.png"/></td><td><a
    href="admin.php?upd=xml">Edit version file</a></td></tr>
    <tr><td><img src="media/icon-save.png"/></td><td><a
    href="admin.php?upd=file">Rewrite version file</a></td></tr>
  </table>

  <?php $upd->provide_update_form(); ?>
  <div style="clear: both;"></div>

<?php

$tpl->printFooter();

?>
