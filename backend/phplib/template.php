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

class Template {

  var $filehandle;
  var $pageTitle;
  var $tplFile;

  function Template($file, $title) {
    $this->pageTitle = $title;
    $this->tplFile = $file;
  }

  function substitute (&$line) {
    $line = preg_replace('/\$TITLE\$/', $this->pageTitle, $line);
    $line = preg_replace('/\$DATE\$/', date("d.m.Y", time()), $line);
    $line = preg_replace('/\$TIME\$/', date("H:i", time()), $line);
  }

  function printHeader() {
    $this->filehandle = fopen('template/'.$this->tplFile, 'r');
    if (!$this->filehandle)
      die("Template not found!");

    while (!feof($this->filehandle)) {
      $line = fgets($this->filehandle, 1024);
      $this->substitute($line);
      if (preg_match('/\$CONTENT\$/', $line))
        return;
      print $line;
    }
  }

  function printFooter() {
    if (!$this->filehandle)
      die("Template not found!");

    while (!feof($this->filehandle)) {
      $line = fgets($this->filehandle, 1024);
      $this->substitute($line);
      print $line;
    }
    fclose($this->filehandle);
  }

}

?>
