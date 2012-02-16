{literal}
<script type="text/javascript">
	function check(box) {
		var boxes = 0;
		files = document.getElementsByName("file[]");
		for (file in files) {
			boxes+=(files[file].checked)?1:0;
		}
		/*if (boxes > 2) {
			box.checked = false;
			boxes--;
		}*/
		but = document.getElementById('cmp_button');
		if (boxes == 2 || boxes == 1) {
			but.disabled = "";
			but.removeAttribute("disabled");
		} else {
			but.disabled = "disabled";
		}
	}
</script>
<script type="text/javascript" src="js/sorttable.js"></script>
{/literal}

<h4>How to use DIFFER in a few steps:</h4>
<ol>
<li> On the File manager tab, upload images you want to analyse and/or compare. Images for the comparation must have same pixel dimensions.</li>
<li> Select 1 image to analyse or 2 images to compare them and press Execute button.</li>
<li> You'll be redirected on the Compare tab. Review the results. </li>
</ol>
<!-- file upload -->
<form enctype="multipart/form-data" action="upload.php" method="POST">
	<input type="hidden" name="MAX_FILE_SIZE" value="50000000" />
	Image file to upload (Max. file size: 15 MB): <input name="uploadedfile" type="file" /><br />
	<b>Supported formats are: TIFF, JPEG, DjVu (only first image is processed in multipage DjVu), JPEG2000.</b><br />
	Website is optimized for Mozilla Firefox 7+.<br />
	<input type="submit" value="Upload File" />
</form>
<!-- images -->
<form action="index.php" method="GET">
	<table class="sortable">
		<tr>
			<th></th>
			<th>file</th>
			<th>extension</th>
			<th>uploaded<span id="sorttable_sortrevind"> &#x25b4;</span></th>
			<th>size (MB)</th>
		</tr>
		{foreach from=$files item=file}
		<tr>
			<td><input type="checkbox" name="file[]" value="{$file.name}" onchange="javascript:check(this);"></td>
			<td>{$file.name}</td>
			<td>&nbsp;&nbsp;&nbsp;&nbsp;{$file.ext}</td>
			<td>&nbsp;&nbsp;{$file.modified}</td>
			<td>&nbsp;&nbsp;&nbsp;{$file.size}</td>
		</tr>
		{/foreach}
	</table>
	<input id="cmp_button" type="submit" name="action" value="EXECUTE" disabled="true"/>
	<input id="delete_images" type="submit" name="action" value="Delete selected images"/>
	<input id="delete_all_files" type="submit" name="action" value="Delete all files" />
	<br />(Only selection of one to two files can be processed, otherwise the Execute button is disabled)
</form>
<form>
	<hr />
{if $profiles|@count gt 0}
	<table class="sortable">
		<tr>
			<th></th>
			<th>file</th>
			<th>modified</th>
		</tr>
	{foreach from=$profiles item=profile}
		<tr>
			<td><input type="checkbox" name="profile[]" value="{$profile.name}"></td>
			<td><a href="profiles/{$profile.name}">{$profile.name}</a></td>
			<td>{$profile.modified}</td>
		</tr>
	{/foreach}
	</table>
	<input id="delete_profiles" type="submit" name="action" value="Delete selected profiles" />
	<input id="delete_all_profiles" type="submit" name="action" value="Delete all profiles" />
{/if}
</form>
<!--
<form action="index.php" method="GET">
	<input type="hidden" name="action" value="delete_all_profiles" />
	<input type="submit" value="Delete all profiles" />
</form>
-->
<!-- error -->
{if $error}
<h1>{$error}</h1>
{/if}
{literal}
<script type="text/javascript">
	check()
</script>
{/literal}
