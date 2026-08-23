#!/bin/bash
cd /home/ferendra-putra/Downloads/EngineSoft/enginotech-cpp/.vscode
# Create extension directory structure
mkdir -p /tmp/engc-extension
cp -r syntaxes /tmp/engc-extension/
cp -r snippets /tmp/engc-extension/
cp extension.json /tmp/engc-extension/package.json
cp language-configuration.json /tmp/engc-extension/
# Create VSIX
cd /tmp
zip -r engc-language-0.3.4.vsix engc-extension/*
mv engc-language-0.3.4.vsix /home/ferendra-putra/Downloads/EngineSoft/enginotech-cpp/dist/
echo "Created VSIX: dist/engc-language-0.3.4.vsix"
