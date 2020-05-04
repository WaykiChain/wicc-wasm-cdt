
   CWD="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
   if [ "${CWD}" != "${PWD}" ]; then
      printf "\\n\\tPlease cd into directory %s to run this script.\\n \\tExiting now.\\n\\n" "${CWD}"
      exit 1
   fi

   BUILD_DIR="${PWD}/build"
   CMAKE_BUILD_TYPE=Release
   TIME_BEGIN=$( date -u +%s )
   INSTALL_PREFIX="/usr/local/wasm.cdt"
   VERSION=1.2

   txtbld=$(tput bold)
   bldred=${txtbld}$(tput setaf 1)
   txtrst=$(tput sgr0)
   
   if [ ! -d "${BUILD_DIR}" ]; then
      printf "\\n\\tError, build.sh has not ran.  Please run ./build.sh first!\\n\\n"
      exit -1
   fi
   if ! pushd "${BUILD_DIR}"; then
      printf "Unable to enter build directory %s.\\n Exiting now.\\n" "${BUILD_DIR}"
      exit 1;
   fi
   if ! make install; then
      printf "\\n\\t>>>>>>>>>>>>>>>>>>>> MAKE installing WASM has exited with the above error.\\n\\n"
      exit -1
   fi
   popd &> /dev/null 

printf "\\tFor more information:\\n"
printf "\\tLLVM website: https://llvm.org\\n"
printf "\\tWASM website: https://github.com/WebAssembly\\n"
