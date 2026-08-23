#!/usr/bin/env python3
"""Fallback: pack ekstensi VSCode EC menjadi .vsix tanpa vsce.
VSIX = ZIP berisi extension/ + [Content_Types].xml + extension.vsixmanifest."""
import json, os, zipfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SRC = os.path.join(ROOT, "tools", "vscode-ec")
DIST = os.path.join(ROOT, "dist")

pkg = json.load(open(os.path.join(SRC, "package.json")))
version = pkg["version"]
ident = pkg["name"]
publisher = pkg["publisher"]
display = pkg.get("displayName", ident)
desc = pkg.get("description", "")
engine = pkg["engines"]["vscode"]

FILES = [
    "package.json", "icon.png", "client.js", "README.md",
    "language-configuration.json", "syntaxes/ec.tmLanguage.json",
    "icons/ec-lang.svg", "server/ec-lsp.js",
]

CONTENT_TYPES = """<?xml version="1.0" encoding="utf-8"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
  <Default Extension="json" ContentType="application/json"/>
  <Default Extension="vsixmanifest" ContentType="text/xml"/>
  <Default Extension="png" ContentType="image/png"/>
  <Default Extension="js" ContentType="application/javascript"/>
  <Default Extension="md" ContentType="text/markdown"/>
</Types>"""

MANIFEST = f"""<?xml version="1.0" encoding="utf-8"?>
<PackageManifest Version="2.0.0" xmlns="http://schemas.microsoft.com/developer/vsx-schema/2011" xmlns:d="http://schemas.microsoft.com/developer/vsx-schema-design/2011">
  <Metadata>
    <Identity Language="en-US" Id="{ident}" Version="{version}" Publisher="{publisher}"/>
    <DisplayName>{display}</DisplayName>
    <Description xml:space="preserve">{desc}</Description>
    <Categories>Programming Languages</Categories>
    <Properties>
      <Property Id="Microsoft.VisualStudio.Code.Engine" Value="{engine}"/>
      <Property Id="Microsoft.VisualStudio.Code.ExtensionDependencies" Value=""/>
      <Property Id="Microsoft.VisualStudio.Code.ExtensionPack" Value=""/>
      <Property Id="Microsoft.VisualStudio.Code.LocalizedLanguages" Value=""/>
      <Property Id="Microsoft.VisualStudio.Code.PreRelease" Value="false"/>
    </Properties>
  </Metadata>
  <Installation>
    <InstallationTarget Id="Microsoft.VisualStudio.Code"/>
  </Installation>
  <Dependencies/>
  <Assets>
    <Asset Type="Microsoft.VisualStudio.Code.Manifest" Path="extension/package.json" Addressable="true"/>
  </Assets>
</PackageManifest>"""

os.makedirs(DIST, exist_ok=True)
out = os.path.join(DIST, f"{ident}-{version}.vsix")
with zipfile.ZipFile(out, "w", zipfile.ZIP_DEFLATED) as z:
    z.writestr("[Content_Types].xml", CONTENT_TYPES)
    z.writestr("extension.vsixmanifest", MANIFEST)
    for rel in FILES:
        z.write(os.path.join(SRC, rel), f"extension/{rel}")
print(f"VSIX: {out}")
