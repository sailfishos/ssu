#!/bin/sh

function repo_clean {
  REPO_DIR=/etc/zypp/repos.d/
  TMP_DIR=${REPO_DIR}/old/

  mkdir -p ${TMP_DIR}

  for REPO_FILE in `ls -1 ${REPO_DIR}/*.repo`; do
    rpm -qf ${REPO_FILE} &> /dev/null
    if [ $? -eq 1 ]; then
      echo "Moving ${REPO_FILE} to ${TMP_DIR}."
      mv ${REPO_FILE} ${TMP_DIR}
    fi
  done
}

if [ $# -ne 1 -o "$1" != "clean" ]; then
  echo "Usage: $0 clean"
  exit 1
fi

repo_clean

