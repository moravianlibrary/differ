<?php

require("config.php");
require("common.php");
$config = config();

$smarty = smarty_init();

$smarty->assign("profiles", $profiles);
$smarty->assign('template', 'help.tpl');
$smarty->assign('selected', 'help');
$smarty->display('layout.tpl');
