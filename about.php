<?php

require("config.php");
require("common.php");
$config = config();

$smarty = smarty_init();

$smarty->assign("profiles", $profiles);
$smarty->assign('template', 'about.tpl');
$smarty->assign('selected', 'about');
$smarty->display('layout.tpl');
