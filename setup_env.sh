if [ -d /opt/uClinux/bfin-uclinux/bin ]; then
  export PATH="/opt/uClinux/bfin-uclinux/bin:$PATH"
else
  export CROSS=""
fi
