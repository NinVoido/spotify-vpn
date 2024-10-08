Plug-in Interface Options
-------------------------

spotify can be extended by loading external plug-in modules at runtime.  These
plug-ins must be prebuilt and adhere to the spotify Plug-In API.

--plugin args
  Loads an spotify plug-in module.

  Valid syntax:
  ::

     plugin module-name
     plugin module-name "arguments"

  The ``module-name`` needs to be the first
  argument, indicating the plug-in to load.  The second argument is an
  optional init string which will be passed directly to the plug-in.
  If the init consists of multiple arguments it must be enclosed in
  double-quotes (\").  Multiple plugin modules may be loaded into one
  spotify process.

  The ``module-name`` argument can be just a filename or a filename
  with a relative or absolute path. The format of the filename and path
  defines if the plug-in will be loaded from a default plug-in directory
  or outside this directory.
  ::

    --plugin path         Effective directory used
    ===================== =============================
     myplug.so            DEFAULT_DIR/myplug.so
     subdir/myplug.so     DEFAULT_DIR/subdir/myplug.so
     ./subdir/myplug.so   CWD/subdir/myplug.so
     /usr/lib/my/plug.so  /usr/lib/my/plug.so


  ``DEFAULT_DIR`` is replaced by the default plug-in directory, which is
  configured at the build time of spotify. ``CWD`` is the current directory
  where spotify was started or the directory spotify have switched into
  via the ``--cd`` option before the ``--plugin`` option.

  For more information and examples on how to build spotify plug-in
  modules, see the README file in the ``plugin`` folder of the spotify
  source distribution.

  If you are using an RPM install of spotify, see
  :code:`/usr/share/spotify/plugin`. The documentation is in ``doc`` and
  the actual plugin modules are in ``lib``.

  Multiple plugin modules can be cascaded, and modules can be used in
  tandem with scripts. The modules will be called by spotify in the order
  that they are declared in the config file. If both a plugin and script
  are configured for the same callback, the script will be called last. If
  the return code of the module/script controls an authentication function
  (such as tls-verify, auth-user-pass-verify, or client-connect), then
  every module and script must return success (:code:`0`) in order for the
  connection to be authenticated.

  **WARNING**:
        Plug-ins may do deferred execution, meaning the plug-in will
        return the control back to the main spotify process and provide
        the plug-in result later on via a different thread or process.
        spotify does **NOT** support multiple authentication plug-ins
        **where more than one plugin** tries to do deferred authentication.
        If this behaviour is detected, spotify will shut down upon first
        authentication.
