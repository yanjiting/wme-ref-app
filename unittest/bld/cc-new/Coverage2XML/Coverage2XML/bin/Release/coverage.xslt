<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:msxsl="urn:schemas-microsoft-com:xslt">
  <xsl:template match="/">
    <xsl:apply-templates select="//CoverageDSPriv" />
  </xsl:template>
  <xsl:template match="CoverageDSPriv">
    <script type="text/javascript">
      <![CDATA[ 
				function toggleSiblingPlusOne()
				{
					if (window.event.srcElement.nextSibling.nextSibling.style.display=="none")
						window.event.srcElement.nextSibling.nextSibling.style.display = "block";
					else
						window.event.srcElement.nextSibling.nextSibling.style.display = "none";
						
				}
  ]]>
    </script>
    <div class="header-title">Sample Code Coverage Report</div>
    <xsl:apply-templates select="Module" />
  </xsl:template>

  <xsl:template match="Module">
    <div style="width:775px;">
    <div style="cursor:pointer;cursor:hand;" onclick="toggleSiblingPlusOne();">
      Module : <xsl:value-of select="ModuleName" />
    </div>
    <div>
        <xsl:call-template name="displayTable">
          <xsl:with-param name="v" select="BlocksCovered" />
          <xsl:with-param name="nv" select="BlocksNotCovered" />
        </xsl:call-template>
      </div>
      <div  style="display:none;">
        <xsl:apply-templates select="NamespaceTable" />
      </div>
      <hr/>
    </div>
  </xsl:template>

  <xsl:template match="NamespaceTable">
    <div style="width:775px;padding-left:25px;">
      <div style="cursor:pointer;cursor:hand;" onclick="toggleSiblingPlusOne();">
        Namespace : <xsl:value-of select="NamespaceName" />
      </div>
      <div>
        <xsl:call-template name="displayTable">
          <xsl:with-param name="v" select="BlocksCovered" />
          <xsl:with-param name="nv" select="BlocksNotCovered" />
        </xsl:call-template>
      </div>
      <div  style="display:none;">
        <xsl:apply-templates select="Class" />
      </div>
      <hr/>
    </div>
  </xsl:template>

  <xsl:template match="Class">
    <div style="width:750px;padding-left:25px;">

      <div style="cursor:pointer;cursor:hand;" onclick="toggleSiblingPlusOne();">
        Class : <xsl:value-of select="ClassName" />
      </div>
      <div>
        <xsl:call-template name="displayTable">
          <xsl:with-param name="v" select="BlocksCovered" />
          <xsl:with-param name="nv" select="BlocksNotCovered" />
        </xsl:call-template>
      </div>
      <div  style="display:none;">
        <xsl:apply-templates select="Method" />
      </div>
      <hr/>
    </div>
  </xsl:template>
  
  <xsl:template match="Method">
    <div style="width:725px;padding-left:25px;">
      <div style="cursor:pointer;cursor:hand;" onclick="toggleSiblingPlusOne();">
        Method : <xsl:value-of select="MethodName" />
      </div>
      <div>
        <xsl:call-template name="displayTable">
          <xsl:with-param name="v" select="BlocksCovered" />
          <xsl:with-param name="nv" select="BlocksNotCovered" />
        </xsl:call-template>
      </div>
      <div  style="display:none;">
        <xsl:apply-templates select="Line" />
      </div>
    </div>
  </xsl:template>

  <xsl:template name="displayTable">
    <xsl:param name="v" />
    <xsl:param name="nv" />
    <xsl:variable name="covered" select="100*($v div ($nv+$v))" />
    <xsl:variable name="notcovered" select="100*($nv div ($nv+$v))" />
    <table style="border:1px solid black;" width="100%" class="section-table">
      <tbody>
        <tr>
          <xsl:if test="$notcovered &gt; 0">
            <td width="{$notcovered}%" style="background-color:red;color:white;text-align:center;">
            <nobr><xsl:value-of select="format-number($notcovered,'0.#')" />%</nobr>
          </td>
          </xsl:if>
          <xsl:if test="$covered &gt; 0">
            <td width="{$covered}%" style="background-color:green;color:white;text-align:center;">
              <nobr>
                <xsl:value-of select="format-number($covered, '0.##')" />%
              </nobr>
          </td>
          </xsl:if>
        </tr>
      </tbody>
    </table>
  </xsl:template>
</xsl:stylesheet>
