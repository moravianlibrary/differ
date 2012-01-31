<!DOCTYPE html>
<html>
	<head>
		<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
		<title>NDK Image Data Validator</title>
		<link type="text/css" href="main.css" rel="stylesheet" />
		<link rel="shortcut icon" href="image/differ_logo_64.ico" />
	
	</head>
	<body><!-- Logo a nadpis -->
		<div style="min-width: 730px;">
			<a href="http://www.nkp.cz/"><img src="image/logo.jpg" height="82px" width="117px" border="none"></a>
			<h1 style="color:white; font-size:45px; display:inline; margin-left: 20px; margin-bottom: 10px;"> NDK Image Data Validator</h1>
		</div>

		<!-- Tabs - Hlavni okno --> 
		<div id="#tabs" class="ui-tabs ui-widget ui-widget-content ui-corner-all" style="clear:both; min-width: 730px;">
		<!-- MENU -->
			<ul id="tabs" class="ui-tabs ui-tabs-nav ui-helper-reset ui-helper-clearfix ui-widget-header ui-corner-all">
				{assign var="sel_class" value="ui-state-default ui-corner-top ui-tabs-selected ui-state-active ui-state-focus ui-state-hover"}
				{assign var="unsel_class" value="ui-state-default ui-corner-top ui-state-hover"}
				<li class='{if $selected=="about"}{$sel_class}{else}{$unsel_class}{/if}'><a href="about.php">About</a></li>
				<li class='{if $selected=="files"}{$sel_class}{else}{$unsel_class}{/if}'><a href="index.php">File Manager</a></li>
				<li class='{if $selected=="compare"}{$sel_class}{else}{$unsel_class}{/if}'><a href="javascript:return false;">Compare</a></li>
				<li class='{if $selected=="profile"}{$sel_class}{else}{$unsel_class}{/if}'><a href="profile.php">Profile Creator</a></li>
				<li class='{if $selected=="faq"}{$sel_class}{else}{$unsel_class}{/if}'><a href="faq.php">FAQ</a></li>
				<li class='{if $selected=="documentation"}{$sel_class}{else}{$unsel_class}{/if}'><a href="doc.php">Documentation</a></li>
				<li class='{if $selected=="traffic"}{$sel_class}{else}{$unsel_class}{/if}'><a href="traffic.php">Traffic</a></li>
				<li class='{if $selected=="help"}{$sel_class}{else}{$unsel_class}{/if}' style="float:right;"><a href="help.php">Help</a></li>
			</ul>
			<div id="#tabs-1" class="divpage ui-tabs-panel ui-widget-content ui-corner-bottom">
				{include file="$template"}
			</div>
		<br class="clear" />
		<br/>
		</div>
		<div align="center">©2011-2012 The National Library of the Czech republic | <a href="tos.php">Terms of Use</a> </div>
	<script type="text/javascript">
	{literal}
	var _gaq = _gaq || [];
		_gaq.push(['_setAccount', 'UA-27726498-1']);
		_gaq.push(['_trackPageview']);
	(function() {
		var ga = document.createElement('script'); ga.type = 'text/javascript'; ga.async = true;
		ga.src = ('https:' == document.location.protocol ? 'https://ssl' : 'http://www') + '.google-analytics.com/ga.js';
		var s = document.getElementsByTagName('script')[0]; s.parentNode.insertBefore(ga, s);
	})();
	{/literal}
	</script>
	</body>
</html>
