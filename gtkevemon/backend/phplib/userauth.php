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

require_once("phplib/template.php");

class UserAuth
{
  private $users;

  public function __construct ()
  {
    $this->init_users();
    $user = isset($_SERVER['PHP_AUTH_USER']) ? $_SERVER['PHP_AUTH_USER'] : "";
    $pass = isset($_SERVER['PHP_AUTH_PW']) ? $_SERVER['PHP_AUTH_PW'] : "";

    if (!$this->is_allowed($user, $pass))
      $this->request_auth();
  }

  private function request_auth ()
  {
    header('WWW-Authenticate: Basic realm="Versioning Backend"');
    header('HTTP/1.0 401 Unauthorized');
    $tpl = new Template("index.tpl", "Versioning backend");
    $tpl->printHeader();
    print("<h3>Access denied</h3>");
    print("<p>You're not allowed to use this service!</p>");
    $tpl->printFooter();
    exit;
  }

  private function is_allowed ($user, $pass)
  {
    if (!isset($user) || $user == "")
      return false;

    foreach ($this->users as $up)
      if ($up[0] == $user && $up[1] == md5($pass))
        return true;

    return false;
  }

  /* Add new administrative users here with their MD5 passwords. */
  private function init_users ()
  {
    $this->users = array();
    $this->add_user("username", "md5passmd5passmd5passmd5passmd5p");
  }

  private function add_user ($user, $pass)
  {
    array_push($this->users, array($user, $pass));
  }
}

?>
