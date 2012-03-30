                                           
    <div id="summary">
<!-- ************************************************************* -->
		
<h2>DIFFER (Determinator of Image File Format propERties)</h2>

<P>We
are pleased to introduced a developing application called DIFFER
(tool wrapper with web based GUI) designed to help to identificate,
validate, and characterize selected still image file formats.</P>
<P>This
application supports selected still image formats: JPEG/JFIF (*.jpe,
*.jpeg), JPEG2000 (*.jp2, *.jpf, *. jpx), TIFF (*.tif, *.tiff), DjVu,
sDjVu (*.djv, *.djvu), PNG (*.png) with
the maximum file size 15&nbsp;MB. The selection was not random; it is
based on needs of the Digital Preservation Standard Department at the
National Library in the Czech Republic.</P>
<P>Identification,
validation, and characterization actions are frequently necessary
during routine operation of digital repositories and for digital
preservation activities. We use for these actions different existing
tools and modules:
<UL>
	<LI />DROID	(Digital Record Object Identification) 
			<A HREF="http://droid.sourceforge.net/">http://droid.sourceforge.net/</A>
	<LI />ExifTool (Read, Write and Edit Meta Information) 
			<A HREF="http://www.sno.phy.queensu.ca/~phil/exiftool/">http://www.sno.phy.queensu.ca/~phil/exiftool/</A>
	<LI />JHOVE	(JSTOR/Harvard Object Validation Environment)
			<A HREF="http://hul.harvard.edu/jhove/">http://hul.harvard.edu/jhove/</A>
	<LI />MTD	Extraction Tool
			<A HREF="http://meta-extractor.sourceforge.net/">http://meta-extractor.sourceforge.net/</A>
	<LI />PRONOM (The technical registry PRONOM)
			<A HREF="http://www.nationalarchives.gov.uk/PRONOM/Default.aspx">http://www.nationalarchives.gov.uk/PRONOM/Default.aspx</A>
	<LI/ >A simple JP2 file structure checker
			 <A HREF="http://www.openplanetsfoundation.org/blogs/2011-09-01-simple-jp2-file-structure-checker">http://www.openplanetsfoundation.org/blogs/2011-09-01-simple-jp2-file-structure-checker</A>
</UL>
</P>
<P>In
addition, application DIFFER includes the following open source libraries:
<UL>
	<LI />KDU_expand
			<A HREF="http://www.kakadusoftware.com/">http://www.kakadusoftware.com</A>
	<LI />LibTIFF
			<A HREF="http://www.libtiff.org/">http://www.libtiff.org/</A>
	<LI />DJVUDUMP
			 <A HREF="http://djvu.sourceforge.net/doc/man/djvudump.html">http://djvu.sourceforge.net/doc/man/djvudump.html</A>
</UL>
</p>

<P>
The project&nbsp;DIFFER is built&nbsp;in order
to&nbsp;implement&nbsp;only&nbsp;open-source&nbsp;tools
and&nbsp;modules.&nbsp;The whole&nbsp;wrapper&nbsp;is intended&nbsp;for
free use&nbsp;and is itself an&nbsp;open-source.
</P>

<P>Format <EM>identification</EM> is the process of determining the format to which a digital object
conforms; in other words, it answers the question: <I>&quot;I
have a digital object; what format is it?&quot;</I><sup>1</sup>
</P>
<P>Format <EM>validation</EM> is the process of determining the level of compliance of a digital
object to the specification for its purported format, e.g.: <I>&quot;I
have an object purportedly of format F, is it?&quot;</I><sup>1</sup></P>

<P>Format validation conformance is determined at two
levels: <I>well-formedness</I>&nbsp;and&nbsp;<I>validity</I>.<sup>1</sup>
<OL>
	<LI />A
	digital object is well-formed if it meets the purely syntactic
	requirements for its format.
	<LI />An
	object is valid if it is well-formed and it meets additional
	semantic-level requirements.
</OL>

<sup>1</sup> <A HREF="http://hul.harvard.edu/jhove/">http://hul.harvard.edu/jhove/</A>
</P>

<P>Format characterization is
the process of determining the format-specific significant properties
of an object of a given format, e.g.: <I>&quot;I
have an object of format F, what are its salient properties?&quot;</I>
</P>

<P><b>SYSTEM
REQUIREMENTS:</b><br />
<ul><li />PHP
+ Apache2
<li />C++ (g++) compiler
<li />Disk space about 20 GB
<li />RAM minimum 2GB, optimal 4GB
</ul>
</P>

<P><b>PROJECT TEAM:</b><br /> <br />

<b>Project
manager</b><br />
Bedrich
Vychodil, Head of Digital preservation Standards Department at
National Library of the Czech Republic, <br />contact:
bedrich.vychodil@nkp.cz, bedrich@gmail.com<br /> <br />

<B>Programmer</b><br />
Vaclav Rosecky<br />
<br /> <br />

<b>Tester</b><br />
Jana Bouckova, External staff<br /> <br />

<b>Consultant</b><br />
Petr Stolcpart, External staff<br />
</P>


		
		
		
		
<!-- ///////////////////////////////////////////////////////////// -->		
    </div>