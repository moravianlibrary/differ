<html>
  <head>
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8"/>
    <title>Histogram</title>
    <script language="javascript" type="text/javascript" src="js/jquery.js"></script>
    <script language="javascript" type="text/javascript" src="js/jquery.flot.js"></script>
    <script language="javascript" type="text/javascript" src="js/jquery.flot.navigate.js"></script>
    <script language="javascript" type="text/javascript" src="js/utils.js"></script>
    <script language="javascript" type="text/javascript">
    {literal}
      function switch_cvs(id) {
         cvs = document.getElementById(id);
         label = document.getElementById('cvs_label');
         if (cvs.style.display == 'none') {
            cvs.style.display = 'block';
            label.innerHTML = 'hide';
         } else {
            cvs.style.display = 'none';
            label.innerHTML = 'display';
         }
      }
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
      function validate(checkbox, elm, name) {
         $.ajax({
            url: "compare.php?op=validate&file[]=" + encodeURIComponent(name),
            context: document.body,
            success: function(data){
               if (checkbox != null) {
                  $(checkbox).attr("style", "visibility:hidden;");
               }
               var html = "";
               $(data).find("result").each(function(){
                  html += "<table>";
                  type = $(this).attr("type");
                  // html += "<tr><td>type:</td><td><b>" + type + "</b></td></tr>";
                  html += "<tr colspan='2'><td><b>" + type + "</b></td></tr>";
                  $(this).children().each(function(){
                    key = this.nodeName;
                    tool = $(this).attr("tool");
                    status = $(this).attr("status");
                    var value = $(this).text();
                    if (value.length > 32) {
                       value = "<abbr title='" + value + "'>" + value.substring(0, 32) + "</abbr>";
                    }
                    if (status != "CONFLICT") {
                       html += "<tr><td>&nbsp;&nbsp;" + key + "</td><td>" + tool + "</td><td>" + value + "</td></tr>";
                    } else {
                       html += "<tr style='background:pink;'><td>&nbsp;&nbsp;" + key + "</td><td>" + tool + "</td><td>" + value + "</td></tr>";
                    }
                  });
                  html += "</table>";
               });
               status = $(data).find("status").text();
               format = $(data).find("format").text();
               djvu = $(data).find("djvu_dump").text();
               if (djvu) {
                  html += "<textarea cols='40' rows='25' onClick='javascript:this.focus(); this.select();'>";
                  html += djvu;
                  html += "</textarea>";
               }
               $("#" + elm + "_validation").html(html);
            }
         });
      }
      function jp2_profile(checkbox, elm, name, profile) {
         var translate = {
            "Clevels" : "Decomposition levels<br/>&nbsp;&nbsp;(Clevels)",
            "Clayers" : "Quality layers<br/>&nbsp;&nbsp;(Clayers)",
            "Corder" : "Progression order<br/>&nbsp;&nbsp;(Corder)",
            "Creversible" : "Compression<br/>&nbsp;&nbsp;(Creversible)",
            "Ckernels" : "Transformation<br/>&nbsp;&nbsp;(Ckernels)",
            "Cblk" : "Code block size<br/>&nbsp;&nbsp;(Cblk)",
            "Cuse_precints" : "Regions of interests<br/>&nbsp;&nbsp;(Cuse_precints)",
            "Cuse_sop" : "Start of packet header<br/>&nbsp;&nbsp;(Cuse_sop)",
            "Cuse_eph" : "End of packet header<br/>&nbsp;&nbsp;(Cuse_eph)",
         };
         $(checkbox).attr("disabled", true);
         $.ajax({
            url: "compare.php?op=jp2_profile&file[]=" + encodeURIComponent(name) + "&profile=" + encodeURIComponent(profile),
            context: document.body,
            success: function(data) {
               var html = "<tr><td>Profile</td><td>Compared profile</td><td>Actual file values</td></tr>";
               $(data).find("metadata").each(function() {
                  valid = $(this).attr("valid");
                  if (valid == 1) {
                     html += "<tr style='background-color: LightGreen';>";
                  } else {
                     html += "<tr style='background-color: Pink';>";
                  }
                  var key = $(this).attr("key");
                  if (translate[key]) {
                     key = translate[key];
                  }
                  html += "<td>" + "&nbsp;&nbsp;&nbsp;" + key + "&nbsp;<a href='help.html#" + $(this).attr("key") + "'>(?)</a>" + "</td>";
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
      //
      var r1 = [ {foreach from=$diff.histogram1.r key=k item=v} [ {$k}, {$v} ], {/foreach} ];
      var g1 = [ {foreach from=$diff.histogram1.g key=k item=v} [ {$k}, {$v} ], {/foreach} ];
      var b1 = [ {foreach from=$diff.histogram1.b key=k item=v} [ {$k}, {$v} ], {/foreach} ];
      var r2 = [ {foreach from=$diff.histogram2.r key=k item=v} [ {$k}, {$v} ], {/foreach} ];
      var g2 = [ {foreach from=$diff.histogram2.g key=k item=v} [ {$k}, {$v} ], {/foreach} ];
      var b2 = [ {foreach from=$diff.histogram2.b key=k item=v} [ {$k}, {$v} ], {/foreach} ];
   {literal}
      // log histogram
      function draw_log_histogram() {
        $.plot($("#placeholder_log_histogram"), [ { color: "red", data: r_log }, { color: "green", data: g_log }, { color: "blue", data: b_log },  ],
          { yaxis: { min: 0, max: 20 }, xaxis: { min: 0, max: 256 }/*, zoom: { interactive: true }, pan: { interactive: true }*/ }
        );
      }
      draw_log_histogram();
      function redraw_log_histogram() {
        x_max = self.document.forms.log_histogram.x.value;
        y_max = self.document.forms.log_histogram.y.value;
        $.plot($("#placeholder_log_histogram"), [ { color: "red", data: r_log}, { color: "green", data: g_log }, { color: "blue", data: b_log} ],
          { yaxis: { min: 0, max: y_max, zoomRange: [0.1, 10] }, xaxis: { min: 0, max: x_max, zoomRange: [0.1, 10] }, zoom: { interactive: true } }
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
        $.plot($("#placeholder_histogram"), [ { color: "red", data: r}, { color: "green", data: g }, { color: "blue", data: b} ],
          { yaxis: { min: 0, max: y_max }, xaxis: { min: 0, max: x_max }  }
        );
        $.plot($("#placeholder_histogram1"), [ { color: "red", data: r1}, { color: "green", data: g1 }, { color: "blue", data: b1} ],
          { yaxis: { min: 0, max: y_max }, xaxis: { min: 0, max: x_max }  }
        );
        $.plot($("#placeholder_histogram2"), [ { color: "red", data: r2}, { color: "green", data: g2 }, { color: "blue", data: b2} ],
          { yaxis: { min: 0, max: y_max }, xaxis: { min: 0, max: x_max }  }
        );
        return false;
      }
      function draw_histograms() {
        draw_histogram1();
        draw_histogram();
        draw_log_histogram()
        draw_histogram2();
      }
      function draw_histogram1() {
        $.plot($("#placeholder_histogram1"), [ { color: "red", data: r1}, { color: "green", data: g1 }, { color: "blue", data: b1 }, ]
          // { yaxis: { min: 0, max: 20 }, xaxis: { min: 0, max: 256 }  } 
        );
      }
      draw_histogram1();
      function draw_histogram2() {
        $.plot($("#placeholder_histogram2"), [ { color: "red", data: r2}, { color: "green", data: g2 }, { color: "blue", data: b2 }, ]
          // { yaxis: { min: 0, max: 20 }, xaxis: { min: 0, max: 256 }  } 
        );
      }
      draw_histogram2();
    {/literal}
    </script>
    <h1>Data processing</h1>
    <div id="summary">
       <table>
          <tr>
             <td>CPU time</td>
             <td>{$cputime}</td>
          </tr>
       </table>
    </div>
    <div id="table">
      {foreach from=$images item=image name=image}
      <table align="left" style="border-right: 1px solid black; padding: 5px; word-wrap:break-word;">
        <tr>
          <td colspan="2">File: {$image.name}</td>
        </tr>
        <tr>
          <td colspan="2">
            <a href="{$image.big}"><img src="{$image.thumbnail}" border="1"/></a>
          </td>
        </tr>
        <tr>
          <td>Hash:</td>
          <td>{$image.hash}</td>
        </tr>
        <tr>
          <td>Linear histogram</td>
        </tr>
        <tr>
        </tr>
        <tr>
          <td colspan="2">
            <div id="placeholder_histogram{$smarty.foreach.image.iteration}" style="width:350px;height:200px;">
          </td>
        </tr>
                <tr>
          <td><b>Export histogram in CVS</b></td>
        </tr>
        <tr>
          <td>
            <a id="cvs_label{$smarty.foreach.image.iteration}" href="javascript:switch_cvs('cvs{$smarty.foreach.image.iteration}');">display</a>
          </td>
        </tr>
        <tr>
          <td colspan="2">
            <div id="cvs{$smarty.foreach.image.iteration}" style="display:none;">
              <textarea cols="40" rows="32" onClick="javascript:this.focus(); this.select();" id="cvs_area">
{if $smarty.foreach.image.iteration == 0}
{foreach from=$cvs_export1 item=rgb name=rgb}
{$rgb.r},{$rgb.g},{$rgb.b}
{/foreach}
{else}
{foreach from=$cvs_export2 item=rgb name=rgb}
{$rgb.r},{$rgb.g},{$rgb.b}
{/foreach}
{/if}
              </textarea>
            </div>
          </td>
        </tr>
        <!--
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
        -->
        <tr>
          <td colspan="2">
            &nbsp;Validate<a href="help.html#validate">(?)</a>
            <!--
            <script>
              $(document).ready(function() {ldelim}
                validate(null, 'image{$smarty.foreach.image.iteration}', '{$image.name}');
              {rdelim});
            </script>
            -->
            <img src="image/loadingAnimation.gif" onLoad="validate(this, 'image{$smarty.foreach.image.iteration}', '{$image.name}');"/>
            <!--
            <input autocomplete="off" type="checkbox" onClick="validate(this, 'image{$smarty.foreach.image.iteration}', '{$image.name}');">&nbsp;Validate <a href="help.html#validate">(?)</a>:</input>
            -->
          </td>
        </tr>
        <tr>
          <td colspan="2">
            <div id="image{$smarty.foreach.image.iteration}_validation">
          </td>
        </tr>
        <!--
        <tr>
          <td>&nbsp;&nbsp;&nbsp;Format:</td>
          <td id="image{$smarty.foreach.image.iteration}_format"></td>
        </tr>
        <tr>
          <td>&nbsp;&nbsp;&nbsp;Status:</td>
          <td id="image{$smarty.foreach.image.iteration}_status"></td>
        </tr>
        -->
        {if $image.format == "JPEG2000"}
          {foreach from=$profiles item=profile name=profile}
            <tr>
              <td colspan="2">
                <input autocomplete="off" type="checkbox" onClick="jp2_profile(this, 'image{$smarty.foreach.image.iteration}_jp2_profile_{$smarty.foreach.profile.iteration}', '{$image.name}', '{$profile}');">&nbsp;Profile validation ({$profile}) <a href="help.html#jp2_validation">(?)</a>:</input>
              </td>
            </tr>
            <tr>
              <td colspan="2" id="image{$smarty.foreach.image.iteration}_jp2_profile_{$smarty.foreach.profile.iteration}"></td>
            </tr>
          {/foreach}
        {/if}
      </table>
      {/foreach}
      {if $compare}
      <table align="left" style="padding: 5px;">
        <tr>
          <td>Comparison:</td>
        </tr>
        <tr>
          <td colspan="2">
            <a href="{$diff.big}"><img src="{$diff.thumbnail}" border="1"/></a>
          </td>
        </tr>
        <tr>
          <td>Equal HASH:</td>
          <td>{$diff.equal}</td>
        </tr>
        <tr>
          <td>Linear histogram</td>
        </tr>
        <tr>
          <td colspan="2">
            <div id="placeholder_histogram" style="width:350px;height:200px;"/>
          </td>
        </tr>
        <tr>
          <td><b>Export histogram in CVS</b></td>
        </tr>
        <tr>
          <td>
            <a id="cvs_label" href="javascript:switch_cvs('cvs');">display</a>
          </td>
        </tr>
        <tr>
          <td>
            <div id="cvs" style="display:none;">
              <textarea cols="40" rows="32" onClick="javascript:this.focus(); this.select();" id="cvs_area">
{foreach from=$cvs_export item=rgb name=rgb}
{$rgb.r},{$rgb.g},{$rgb.b}
{/foreach}
              </textarea>
            </div>
          </td>
        </tr>
        <tr>
          <td>
            <form onSubmit="javascript:redraw_histogram(); return false;" name="histogram"><label>x:</label>&#xA0;<input type="string" name="x" value="256"/><br/><label>y:</label>&#xA0;<input type="string" name="y" value="5000000"/><input type="submit" value="redraw"/></form>
          </td>
        </tr>
        <tr><td>Logarithmic histogram</td></tr>
        <tr>
          <td colspan="2">
            <div id="placeholder_log_histogram" style="width:350px;height:200px;"/>
          </td>
        </tr>
        <tr>
          <td>
            <form onSubmit="javascript:redraw_log_histogram(); return false;" name="log_histogram"><label>x:</label>&#xA0;<input type="string" name="x" value="256"/><br/><label>y:</label>&#xA0;<input type="string" name="y" value="20"/><input type="submit" value="redraw"/></form>
          </td>
        </tr>
        <tr>
          <td>PSNR:</td> 
          {if $diff.psnr.avg == "inf" || $diff.psnr.avg == "nan"}
          <td>infinity dB</td>
          {else}
          <td>{$diff.psnr.avg|string_format:"%.2f"} dB</td>
          {/if}
        </tr>
        <tr bgcolor="pink">
          <td>PSNR (R):</td>
          {if $diff.psnr.r == "inf"}
          <td>infinity dB</td>
          {else}
          <td>{$diff.psnr.r|string_format:"%.2f"} dB</td>
          {/if}
        </tr>
        <tr bgcolor="lightgreen">
          <td>PSNR (G):</td>
          {if $diff.psnr.g == "inf"}
          <td>infinity dB</td>
          {else}
          <td>{$diff.psnr.g|string_format:"%.2f"} dB</td>
          {/if}
        </tr>
        <tr bgcolor="lightblue">
          <td>PSNR (B):</td>
          {if $diff.psnr.b == "inf"}
          <td>infinity dB</td>
          {else}
          <td>{$diff.psnr.b|string_format:"%.2f"} dB</td>
          {/if}
        </tr>
      </table>
      {/if}
    </div>
    <br clear="all"/>
    <center>
      <a href="help.html">help</a>
    <center>
  </body>
</html>
