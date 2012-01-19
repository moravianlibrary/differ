<?php

require("config.php");
require("common.php");
$config = config();

$smarty = smarty_init();

$smarty->assign("profiles", $profiles);
$smarty->assign('template', 'tos.tpl');
$smarty->assign('selected', 'tos');
$smarty->display('layout.tpl');
