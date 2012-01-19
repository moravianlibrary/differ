<html>
  <head>
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8"/>
    <title>Histogram</title>
    <script language="javascript" type="text/javascript" src="js/jquery.js"></script>
    <script language="javascript" type="text/javascript" src="js/jquery.flot.js"></script>
    <script language="javascript" type="text/javascript" src="js/utils.js"></script>
    <script language="javascript" type="text/javascript">
    {literal}
      /*function switch_cvs() {
         cvs = document.getElementById('cvs');
         label = document.getElementById('cvs_label');
         if (cvs.style.display == 'none') {
            cvs.style.display = 'block';
            label.innerHTML = 'skr&#xFD;t';
         } else {
            cvs.style.display = 'none';
            label.innerHTML = 'zobrazit';
         }
      }*/
      //jhove('', '1984.jpg');
      //alert("hello");
      function jhove(checkbox, elm, name) {
         $(checkbox).attr("disabled", true);
         $.ajax({
            url: "compare.php?op=jhove&file[]=" + encodeURIComponent(name),
            context: document.body,
            success: function(data){
               status = $(data).find("status").text();
               format = $(data).find("format").text();
               $("#" + elm + "_status").html(status);
               $("#" + elm + "_format").html(format);
            }
         });
      }
      function jp2_profile(checkbox, elm, name, profile) {
         $(checkbox).attr("disabled", true);
         $.ajax({
            url: "compare.php?op=jp2_profile&file[]=" + encodeURIComponent(name) + "&profile=" + encodeURIComponent(profile),
            context: document.body,
            success: function(data) {
               var html = "<tr><td>parameter</td><td>profile</td><td>value</td></tr>";
               $(data).find("metadata").each(function() {
                  valid = $(this).attr("valid");
                  if (valid == 1) {
                     html += "<tr style='background-color: green';>";
                  } else {
                     html += "<tr style='background-color: red';>";
                  }
                  html += "<td>" + "&nbsp;&nbsp;&nbsp;" + $(this).attr("key") + "</td>";
                  html += "<td>" + $(this).attr("expected") + "</td>";
                  html += "<td>" + $(this).attr("value") + "</td>";
                  html += "</tr>";
               });
               var tr = $("<table>").html(html);
               $("#" + elm).append(tr);
            }
         });
      }
    {/literal}
    </script>
  </head>
  <!-- <body onload="document.getElementById('cvs_area').value = document.getElementById('cvs_area').value.replace(/;/g, '\n'); "> -->
  <body onload="javascript:draw_histograms();">
    <script type="text/javascript">
      var r_log = [ {foreach from=$diff.histogram.r key=k item=v} [ {$k}, Math.log({$v}) ], {/foreach} ];
      var g_log = [ {foreach from=$diff.histogram.g key=k item=v} [ {$k}, Math.log({$v}) ], {/foreach} ];
      var b_log = [ {foreach from=$diff.histogram.b key=k item=v} [ {$k}, Math.log({$v}) ], {/foreach} ];
      var r = [ {foreach from=$diff.histogram.r key=k item=v} [ {$k}, {$v} ], {/foreach} ];
      var g = [ {foreach from=$diff.histogram.g key=k item=v} [ {$k}, {$v} ], {/foreach} ];
      var b = [ {foreach from=$diff.histogram.b key=k item=v} [ {$k}, {$v} ], {/foreach} ];
   {literal}
      // log histogram
      function draw_log_histogram() {
        $.plot($("#placeholder_log_histogram"), [ { color: "red", data: r_log}, { color: "green", data: g_log }, { color: "blue", data: b_log }, ],
          { yaxis: { min: 0, max: 20 }, xaxis: { min: 0, max: 256 }  } 
        );
      }
      draw_log_histogram();
      function redraw_log_histogram() {
        x_max = self.document.forms.log_histogram.x.value;
        y_max = self.document.forms.log_histogram.y.value;
        $.plot($("#placeholder_log_histogram"), [ { color: "red", data: r_log}, { color: "green", data: g_log }, { color: "blue", data: b_log} ],
          { yaxis: { min: 0, max: y_max }, xaxis: { min: 0, max: x_max }  }
        );
        return false;
      }
      // normal histogram
      function draw_histogram() {
        $.plot($("#placeholder_histogram"), [ { color: "red", data: r}, { color: "green", data: g }, { color: "blue", data: b }, ]
          // { yaxis: { min: 0, max: 20 }, xaxis: { min: 0, max: 256 }  } 
        );
      }
      draw_histogram();
      function redraw_histogram() {
        x_max = self.document.forms.histogram.x.value;
        y_max = self.document.forms.histogram.y.value;
        $.plot($("#placeholder_histogram"), [ { color: "red", data: r}, { color: "green", data: g }, { color: "blue", data: b} ]
          //{ yaxis: { min: 0, max: y_max }, xaxis: { min: 0, max: x_max }  }
        );
        return false;
      }
      function draw_histograms() {
        draw_histogram();
        draw_log_histogram()
      }
    {/literal}
    </script>
    <h1>Data processing</h1>
    <div id="table">
      {foreach from=$images item=image name=image}
      <table align="left" style="border-right: 1px solid black; padding: 5px; word-wrap:break-word;">
        <tr>
          <td>File A:</td>
        </tr>
        <tr>
          <td colspan="2">
            <img src="{$image.thumbnail}"/>
          </td>
        </tr>
        <tr>
          <td>Hash:</td>
          <td>{$image.hash}</td>
        </tr>
        <tr>
          <td>Height:</td>
          <td>{$image.height}</td>
        </tr>
        <tr>
          <td>Width:</td>
          <td>{$image.width}</td>
        </tr>
        <tr>
          <td>Format:</td>
          <td>{$image.format}</td>
        </tr>
        <tr>
          <td colspan="2">
            <input autocomplete="off" type="checkbox" onClick="jhove(this, 'image{$smarty.foreach.image.iteration}', '{$image.name}');">&nbsp;Jhove validation:</input>
          </td>
        </tr>
        <tr>
          <td>&nbsp;&nbsp;&nbsp;Format:</td>
          <td id="image{$smarty.foreach.image.iteration}_format"></td>
        </tr>
        <tr>
          <td>&nbsp;&nbsp;&nbsp;Status:</td>
          <td id="image{$smarty.foreach.image.iteration}_status"></td>
        </tr>
        {if $image.format == "JPEG2000"}
          {foreach from=$profiles item=profile name=profile}
            <tr>
              <td colspan="2">
                <input autocomplete="off" type="checkbox" onClick="jp2_profile(this, 'image{$smarty.foreach.image.iteration}_jp2_profile', '{$image.name}', '{$profile}');">&nbsp;Profile validation:</input>
              </td>
            </tr>
            <tr>
              <td colspan="2" id="image{$smarty.foreach.image.iteration}_jp2_profile"></td>
            </tr>
          {/foreach}
        {/if}
      </table>
      {/foreach}
      <table align="left">
        <tr>
          <td>Comparison:</td>
        </tr>
        <tr>
          <td colspan="2">
            <img src="{$diff.thumbnail}"/>
          </td>
        </tr>
        <tr>
          <td>PSNR:</td>
          <td>{$diff.psnr.avg}</td>
        </tr>
        <tr bgcolor="red">
          <td>PSNR (R):</td>
          <td>{$diff.psnr.r}</td>
        </tr>
        <tr bgcolor="green">
          <td>PSNR (G):</td>
          <td>{$diff.psnr.g}</td>
        </tr>
        <tr bgcolor="blue">
          <td>PSNR (B):</td>
          <td>{$diff.psnr.b}</td>
        </tr>
        <tr>
          <!-- <br/> -->
        </tr>
        <tr>
          <td>Logarithmic histogram</td>
        </tr>
        <tr>
          <td colspan="2">
            <div id="placeholder_log_histogram" style="width:350px;height:200px;"/>
          </td>
        </tr>
        <tr>
          <!-- <br/> -->
        </tr>
        <tr>
          <td>
            <form onSubmit="javascript:redraw_log_histogram(); return false;" name="log_histogram"><label>x:</label>&#xA0;<input type="string" name="x" value="256"/><br/><label>y:</label>&#xA0;<input type="string" name="y" value="20"/><input type="submit" value="redraw"/></form>
          </td>
        </tr>
        <tr>
          <td>Histogram</td>
        </tr>
        <tr>
          <td colspan="2">
            <div id="placeholder_histogram" style="width:350px;height:200px;"/>
          </td>
        </tr>
        <tr>
          <td>
            <form onSubmit="javascript:redraw_histogram(); return false;" name="histogram"><label>x:</label>&#xA0;<input type="string" name="x" value="256"/><br/><label>y:</label>&#xA0;<input type="string" name="y" value="20"/><input type="submit" value="redraw"/></form>
          </td>
        </tr>
        <!--
        <tr>
          <td>Export v CVS</td>
        </tr>
        <tr>
          <td>
            <a id="cvs_label" href="javascript:switch_cvs();">zobrazit</a>
          </td>
        </tr>
        <tr>
          <td>
            <div id="cvs" style="display:none;">
              <textarea cols="40" rows="256" onClick="javascript:this.focus(); this.select();" id="cvs_area">
              </textarea>
            </div>
          </td>
        </tr>
       -->
      </table>
    </div>
    <br clear="all"/>
  </body>
</html>
