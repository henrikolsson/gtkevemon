<?php
/*
 * This file is part of GtkEveMon.
 * Written by Simon Fuhrmann
 *
 * GtkEveMon is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

class Updater
{
  private $vxml;

  public function __construct ($vxml)
  {
    $this->vxml = $vxml;
    $this->check_file_update();
    $this->check_app_update();
    $this->check_xml_update();
    $this->check_rewrite_file();
  }

  public function provide_update_form ()
  {
    if (isset($_GET["upd"]))
    {
      if ($_GET["upd"] == "data")
        $this->print_df_update_form();
      else if ($_GET["upd"] == "app")
        $this->print_app_update_form();
      else if ($_GET["upd"] == "xml")
        $this->print_xml_editor_form();
    }

    if (isset($_FILES['datafile']))
    {
      $this->handle_file_upload();
    }

  }

  private function check_app_update ()
  {
    if (isset($_POST['newapp']))
    {
      print("<h3>Application update</h3>\n");
      $name = $_POST['name'];
      $message = $_POST['message'];
      $source = $_POST['source'];
      $version = chop(implode(file($source)));
      $this->vxml->add_application($name, $version, $message, $source);
      $this->vxml->save_versionfile();
      print("<p>Successfully added <b>$name</b>!</p>\n");
    }

    if (isset($_GET['appupd']))
    {
      print("<h3>Application update</h3>\n");
      $app = $this->vxml->get_application($_GET['appupd']);
      if ($app == NULL)
      {
        print("<p>Error: Could not find application to update!</p>\n");
        return;
      }

      $old_version = $app->version;
      $new_version = chop(implode(file($app->url)));

      if ($old_version == $new_version)
      {
        print("<p>Old and new versions are the same: <b>$new_version</b></p>");
      }
      else
      {
        $this->vxml->add_application($app->name, $new_version,
            $app->message, $app->url);
        $this->vxml->save_versionfile();
        print("<p>Successfully updated <b>$app->name</b> from "
            ."<b>$old_version</b> to <b>$new_version</b>!</p>\n");
      }
    }
  }

  private function check_file_update ()
  {
    if (!isset($_POST['dfupdate']))
      return;

    print("<h3>Data file update</h3>\n");

    $source = "temp/".$_POST['filename'];
    $dest = $_POST['filename'];
    $message = $_POST["message"];

    if (isset($_POST['cancel']))
    {
      unlink($source);
      print("<p>Operation canceled. Your uploaded file has been removed.</p>");
      return;
    }

    $version = $this->get_datafile_version($source);
    if ($version == NULL)
    {
      print("Sorry, couldn't find version information in the XML!<br/>\n");
      unlink($source);
      return;
    }

    if (file_exists($dest))
      unlink($dest);

    if (!rename($source, $dest))
    {
      print("Sorry, the uploaded file could not be applied.<br/>\n");
      unlink($source);
      return;
    }

    $this->vxml->add_datafile($dest, $version, $message);
    $this->vxml->save_versionfile();

    print("<p><b>$dest</b> has been successfully updated!</p>\n");
  }

  private function check_xml_update ()
  {
    if (!isset($_POST['xmltext']))
      return;

    print("<h3>Manual XML update</h3>");

    if (get_magic_quotes_gpc())
      $data = stripslashes($_POST['xmltext']);
    else
      $data = $_POST['xmltext'];

    $this->vxml->save_versionfile_data($data);
    //print("<textarea>".$data."</textarea>");
    print("<p>Version information has been saved!</p>\n");
    $this->vxml->read_versionfile();
  }

  private function check_rewrite_file ()
  {
    if (!isset($_GET['upd']) || $_GET['upd'] != "file")
      return;

    $this->vxml->save_versionfile();
    $this->vxml->read_versionfile();
  }

  private function get_datafile_version ($fname)
  {
    $dom = new DOMDocument();

    if (substr($fname, -3) == ".gz")
    {
      $data = implode(gzfile($fname));
      $dom->loadXML($data);
    }
    else
    {
      $dom->load($fname);
    }

    $tags = array();
    array_push($tags, $dom->getElementsByTagName("eveapi"));
    array_push($tags, $dom->getElementsByTagName("ItemDB"));

    $version = NULL;
    for ($i = 0; $i < count($tags) && !$version; $i++)
    {
      if ($tags[$i]->length > 0)
      {
        $attrib = $tags[$i]->item(0)->attributes->getNamedItem("dataVersion");
        if ($attrib != NULL)
          $version = $attrib->nodeValue;
      }
    }

    return $version;
  }

  private function handle_file_upload ()
  {
    print("<h3>Data file uploaded</h3>\n");
    //print_r($_FILES);
    $name = $_FILES['datafile']['name'];
    $file = $_FILES['datafile']['tmp_name'];
    $error = $_FILES['datafile']['error'];
    $target = "temp/$name";

    if ($error)
    {
      print("There was an error while uploading the file. "
          ."It's probably too big?<br/>\n");
      $this->print_df_update_form();
      return;
    }

    if (!is_dir('temp'))
      if (!mkdir('temp'))
        die("Error creating temp directory!");

    if (!move_uploaded_file($file, $target))
      die("Temporary uploaded file could not be moved!");

    print("You uploaded: <b>".$name."</b><br/>\n");

    if (substr($name, -4) != ".xml" && substr($name, -7) != ".xml.gz")
    {
      print("<b>Error:</b> This file is not a XML file!<br/>\n");
      unlink($target);
      $this->print_df_update_form();
      return;
    }

    $df = $this->vxml->get_datafile($name);
    if ($df != NULL)
    {
      $local_version = $this->get_datafile_version($target);
      if ($local_version == NULL)
      {
        print("Error: Could not determine version of the XML!");
        unlink($target);
        $this->print_df_update_form();
        return;
      }
      print("This data file already exists and it will be UPDATED.<br/>\n");
      print("Current version: ". $df->version
          .", your version: ". $local_version ."<br/>\n");
    }
    else
    {
      print("This data file does not yet exist and it will be CREATED.<br/>\n");
    }

?>
<form action="admin.php" method="post">
  Message: <input type="text" name="message" value="Update info here!"/>
  <input type="hidden" name="dfupdate" value="true"/>
  <input type="hidden" name="filename" value="<?php print $name; ?>"/>
  <input type="submit" name="confirm" value="Continue"/>
  <input type="submit" name="cancel" value="Cancel"/>
</form>
<?php
  }

  private function print_df_update_form ()
  {
?>

<h3>Update a data file</h3>
<form action="admin.php" method="post" enctype="multipart/form-data">
  <input type="file" name="datafile"/>
  <input type="submit" value="Upload"/>
</form>

<?php
  }

  private function print_app_update_form ()
  {
?>

<h3>Update an application</h3>
<?php $this->vxml->print_application_admin(); ?>

<h3>Add a new application</h3>
<form action="admin.php" method="post">
  <input type="hidden" name="newapp" value="true"/>
  <table><tr>
    <td>Application name:</td><td><input type="text" name="name"/></td>
  </tr><tr>
    <td>Version source URL:</td><td><input type="text" name="source"/></td>
  </tr><tr>
    <td>Message:</td><td><input type="text" name="message"/></td>
  </tr><tr>
    <td>&nbsp;</td><td><input type="submit" value="Add application"/></td>
  </tr></table>
</form>

<?php
  }

  private function print_xml_editor_form ()
  {
?>
<h3>Update XML manually</h3>
<form action="admin.php" method="post">
  <textarea name="xmltext" style="width: 640px; height: 200px;"><?php
    $xml = $this->vxml->get_update_xml();
    print($xml);
?></textarea><br/>
  <input type="submit" name="xmledit" value="Save"/>
</form>
<?php
  }
}

?>
