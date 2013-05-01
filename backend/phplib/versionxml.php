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

require_once("phplib/winfixes.php");

class Ressource
{
  public $name;
  public $version;
  public $message;
  public $date;
  public $url;
  public $md5;
  public $size;
}

class VersionXML
{
  private $version_file;

  private $applications;
  private $datafiles;

  public function __construct ($version_file)
  {
    $this->version_file = $version_file;
    $this->applications = array();
    $this->datafiles = array();

    $this->read_versionfile();
  }

  public function add_datafile ($name, $version, $message,
      $date = NULL, $md5 = NULL)
  {
    $df = new Ressource();
    $df->name = $name;
    $df->version = $version;
    $df->message = $message;
    $df->date = $date ? $this->get_ts($date) : time();
    $df->url = "http://gtkevemon.battleclinic.com/updates/$name";
    $df->md5 = $md5 ? $md5 : md5_file($name);
    $df->size = filesize($name);

    $found = false;
    for ($i = 0; $i < count($this->datafiles); $i++)
      if ($this->datafiles[$i]->name == $name)
      {
        $this->datafiles[$i] = $df;
        $found = true;
      }

    if (!$found)
      array_push($this->datafiles, $df);
  }

  public function add_application ($name, $version, $message,
      $url, $date = NULL)
  {
    $app = new Ressource();
    $app->name = $name;
    $app->version = $version;
    $app->message = $message;
    $app->url = $url;
    $app->date = $date ? $this->get_ts($date) : time();

    $found = false;
    for ($i = 0; $i < count($this->applications); $i++)
      if ($this->applications[$i]->name == $name)
      {
        $this->applications[$i] = $app;
        $found = true;
      }

    if (!$found)
      array_push($this->applications, $app);
  }

  public function get_datafile ($name)
  {
    for ($i = 0; $i < count($this->datafiles); $i++)
      if ($this->datafiles[$i]->name == $name)
        return $this->datafiles[$i];
    return NULL;
  }

  public function get_application ($name)
  {
    for ($i = 0; $i < count($this->applications); $i++)
      if ($this->applications[$i]->name == $name)
        return $this->applications[$i];
    return NULL;
  }

  public function read_versionfile ()
  {
    $this->applications = array();
    $this->datafiles = array();

    if (!file_exists($this->version_file))
    {
      $this->save_versionfile();
      return;
    }

    $dom = new DOMDocument();
    $dom->load($this->version_file);

    $apps = $dom->getElementsByTagName("Application");
    $data = $dom->getElementsByTagName("DataFile");

    foreach ($apps as $app)
    {
      $name = $app->getAttribute("name");
      $version = $app->getElementsByTagName("Version")->item(0)->nodeValue;
      $message = $app->getElementsByTagName("Message")->item(0)->nodeValue;
      $url = $app->getElementsByTagName("Source")->item(0)->nodeValue;
      $date = $app->getElementsByTagName("Date")->item(0)->nodeValue;

      $this->add_application($name, $version, $message, $url, $date);
    }

    foreach ($data as $df)
    {
      $name = $df->getAttribute("name");
      $version = $df->getElementsByTagName("Version")->item(0)->nodeValue;
      $message = $df->getElementsByTagName("Message")->item(0)->nodeValue;
      $date = $df->getElementsByTagName("Date")->item(0)->nodeValue;
      $md5 = $df->getElementsByTagName("MD5")->item(0)->nodeValue;

      $this->add_datafile($name, $version, $message, $date, $md5);
    }
  }

  public function save_versionfile ()
  {
    $fh = fopen($this->version_file, "w");
    if (!$fh)
    {
      die("Error: Cannot open $this->version_file for writing!");
    }

    /* Print header. */
    fwrite($fh, '<?xml version="1.0" encoding="UTF-8" ?>'."\n");
    fwrite($fh, '<VersionInformation>'."\n");

    /* Dump all applications. */
    if (count($this->applications) > 0)
    {
      fwrite($fh, '  <Applications>'."\n");
      for ($i = 0; $i < count($this->applications); $i++)
      {
        $app = $this->applications[$i];
        fwrite($fh, '    <Application name="'.$app->name.'">'."\n");
        fwrite($fh, '      <Version>'.$app->version.'</Version>'."\n");
        fwrite($fh, '      <Message>'.$app->message.'</Message>'."\n");
        fwrite($fh, '      <Date>'.$this->get_date($app->date).'</Date>'."\n");
        fwrite($fh, '      <Source>'.$app->url.'</Source>'."\n");
        fwrite($fh, '    </Application>'."\n");
      }
      fwrite($fh, '  </Applications>'."\n");
    }

    /* Dump all data files. */
    if (count($this->datafiles) > 0)
    {
      fwrite($fh, '  <DataFiles>'."\n");
      for ($i = 0; $i < count($this->datafiles); $i++)
      {
        $df = $this->datafiles[$i];
        fwrite($fh, '    <DataFile name="'.$df->name.'">'."\n");
        fwrite($fh, '      <Version>'.$df->version.'</Version>'."\n");
        fwrite($fh, '      <Message>'.$df->message.'</Message>'."\n");
        fwrite($fh, '      <Date>'.$this->get_date($df->date).'</Date>'."\n");
        fwrite($fh, '      <URL>'.$df->url.'</URL>'."\n");
        fwrite($fh, '      <MD5>'.$df->md5.'</MD5>'."\n");
        fwrite($fh, '      <Size>'.$df->size.'</Size>'."\n");
        fwrite($fh, '    </DataFile>'."\n");
      }
      fwrite($fh, '  </DataFiles>'."\n");
    }

    /* Print footer. */
    fwrite($fh, '</VersionInformation>'."\n");

    /* Close output file. */
    fclose($fh);
  }

  public function save_versionfile_data ($data)
  {
    $fh = fopen($this->version_file, "w");
    fwrite($fh, $data);
    fclose($fh);
  }

  public function print_applications ()
  {
    print('<table>');
    foreach ($this->applications as $app)
    {
      print('<tr><td><img src="media/icon-application.png"/></td><td>');
      print("<b>$app->name</b><br/>");
      print("Version: $app->version");
      print('</td></tr>');
    }
    print("</table>\n");
  }

  public function print_datafiles ()
  {
    print('<table>');
    foreach ($this->datafiles as $df)
    {
      print('<tr><td><img src="media/icon-datafile.png"/></td><td>');
      print("<b>$df->name</b> (<a href=\"$df->name\">DL</a>)<br/>");
      print("Version: $df->version, Size: ".$this->get_file_size($df->name));
      print('</td></tr>');
    }
    print("</table>\n");
  }

  public function print_application_admin ()
  {
    print("<table>");
    foreach ($this->applications as $app)
    {
      print("<tr>");
      print("<td>$app->name</td>");
      print('<td><a href="admin.php?appupd='.$app->name.'">Update</a></td>');
      print("</tr>");
    }
    print("</table>");
  }

  public function print_update_xml ()
  {
    $content = $this->get_update_xml();
    print("<pre>".htmlentities($content)."</pre>");
  }

  public function get_update_xml ()
  {
    $xml = fopen($this->version_file, "r");
    $content = fread($xml, filesize($this->version_file));
    fclose($xml);
    return $content;
  }

  public function get_file_size ($name)
  {
    if (!file_exists($name))
      return "<font color='red'>n/a</font>";
    $size = filesize($name);
    if ($size)
      return sprintf("%.2d KB", $size / 1024);
    else
      return "<font color='red'>n/a</font>";
  }

  private function get_date ($timestamp)
  {
    return date("Y-m-d", $timestamp);
  }

  private function get_ts ($date)
  {
    $res = strptime($date, "%Y-%m-%d");
    return mktime(0, 0, 0, $res['tm_mon'] + 1,
        $res['tm_mday'], 1900 + $res['tm_year']);
  }
}

?>
