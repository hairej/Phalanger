/*

 Copyright (c) 2005-2006 Tomas Matousek.
  
 The use and distribution terms for this software are contained in the file named License.txt, 
 which can be found in the root of the Phalanger distribution. By using this software 
 in any fashion, you are agreeing to be bound by the terms of this license.
 
 You must not remove this notice from this software.

*/

using System;
using System.IO;
using System.Text;
using System.Threading;
using System.Reflection;
using System.Diagnostics;
using System.Collections;
using System.Collections.Generic;

using PHP.Core.Reflection;
using PHP.Core.Emit;

#if SILVERLIGHT
using PHP.CoreCLR;
#endif

namespace PHP.Core
{
	/// <summary>
	/// Represents a set of data associated with the current web request targeting PHP scripts.
	/// </summary>
	[Serializable]
	public sealed partial class RequestContext
	{
		#region Fields, Properties, Events

		/// <summary>
		/// Set when the context started finalization.
		/// </summary>
		private bool disposed = false;

		/// <summary>
		/// Current script context.
		/// </summary>
		public ScriptContext/*!*/ ScriptContext { get { return scriptContext; } }
		internal ScriptContext/*!*/ scriptContext;

		/// <summary>
		/// Gets the original value of response encoding set in ASP.NET configuration.
		/// </summary>
		public Encoding DefaultResponseEncoding { get { return defaultResponseEncoding; } }
		private Encoding defaultResponseEncoding;

		/// <summary>
		/// An event fired on the very end of the request. 
		/// </summary>
        public static event Action RequestEnd;

		/// <summary>
		/// An event fired on the beginning of the request after the script context is initialized.
		/// </summary>
        public static event Action RequestBegin;

		#endregion

		#region Request Processing
#if !SILVERLIGHT
		/// <summary>
		/// Performs PHP inclusion on a specified script. Equivalent to <see cref="PHP.Core.ScriptContext.IncludeScript"/>. 
		/// </summary>
		/// <param name="relativeSourcePath">
		/// Path to the target script source file relative to the web application root.
		/// </param>
		/// <param name="script">
		/// Script info (i.e. type called <c>Default</c> representing the target script) or any type from 
		/// the assembly where the target script is contained. In the latter case, the script type is searched in the 
		/// assembly using value of <paramref name="relativeSourcePath"/>.
		/// </param>
		/// <returns>The value returned by the global code of the target script.</returns>
		/// <exception cref="InvalidOperationException">Request context has been disposed.</exception>
		/// <exception cref="ArgumentNullException"><paramref name="relativeSourcePath"/> or <paramref name="script"/> are <B>null</B> references.</exception>
		/// <exception cref="ArgumentException">Script type cannot be resolved.</exception>
		/// <exception cref="InvalidScriptAssemblyException">The target assembly is not a valid Phalanger compiled assembly.</exception>
        [DebuggerNonUserCode]
		public object IncludeScript(string/*!*/ relativeSourcePath, ScriptInfo/*!*/ script)
		{
			if (disposed)
				throw new InvalidOperationException(CoreResources.GetString("instance_disposed"));

            return scriptContext.IncludeScript(relativeSourcePath, script);
		}
#endif

		/// <summary>
		/// Finalizes (disposes) the request context.
		/// </summary>
		/// <remarks>
		/// Finalization comprises of the following actions (executed in the order):
		/// <list type="number">
		/// <term>Output buffers are flushed. This action may include calls to user defined filters (see <c>ob_start</c> function).</term>
		/// <term>Shutdown callbacks are invoked (if added by <c>register_shutdown_function</c> function).</term>
		/// <term>Session is closed. User defined session handling function may be invoked (see <c>session_set_save_handler</c> function).</term>
		/// <term>PHP objects are destroyed.</term>
		/// <term>HTTP Headers are flushed (if it wasn't done earlier).</term>
		/// <term>PHP resources are disposed.</term>
		/// <term>Per-request temporary files are deleted.</term>
		/// <term><see cref="RequestEnd"/> event is fired.</term>
		/// <term>Current request and script contexts are nulled.</term>
		/// </list>
		/// Multiple invocations of the method are ignored.
		/// Since session data need to be written to the session store (<c>HttpContext.Session</c>) this method has to be 
		/// called before the ASP.NET session is ended for the request.
		/// </remarks>
		public void Dispose()
		{
			if (!disposed)
			{   
				try
				{
                    ((IDisposable)scriptContext).Dispose();
				}
				finally
				{
                    if (RequestEnd != null) RequestEnd();

                    // cleans this instance:
					this.disposed = true;
					this.scriptContext = null;

					Debug.WriteLine("REQUEST", "-- disposed ----------------------");
				}
			}
		}

		#endregion
	}
}
