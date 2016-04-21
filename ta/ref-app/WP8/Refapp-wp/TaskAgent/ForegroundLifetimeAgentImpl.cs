/* 
    Copyright (c) 2012 Microsoft Corporation.  All rights reserved.
    Use of this sample source code is subject to the terms of the Microsoft license 
    agreement under which you licensed this sample source code and is provided AS-IS.
    If you did not accept the terms of the license agreement, you are not authorized 
    to use this sample source code.  For the terms of the license, please see the 
    license agreement between you and Microsoft.
  
    To see all Code Samples for Windows Phone, visit http://go.microsoft.com/fwlink/?LinkID=219604 
  
*/
using Microsoft.Phone.Networking.Voip;
using System.Diagnostics;
using System.Threading;

namespace TaskAgent
{
    /// <summary>
    /// An agent that is invoked when the UI process calls Microsoft.Phone.Networking.Voip.VoipBackgroundProcess.Launched()
    /// and is canceled when the UI leaves the foreground.
    /// </summary>
    public sealed class ForegroundLifetimeAgentImpl : VoipForegroundLifetimeAgent
    {
        public ForegroundLifetimeAgentImpl()
            : base()
        {
        }

        /// <summary>
        /// A method that is called as a result of 
        /// </summary>
        protected override void OnLaunched()
        {
            Debug.WriteLine("[ForegroundLifetimeAgentImpl] The UI has entered the foreground.");

            // Indicate that an agent has started running
            //base.NotifyComplete();
        }

        protected override void OnCancel()
        {
           // Debug.WriteLine("[ForegroundLifetimeAgentImpl] The UI is leaving the foreground");
            base.NotifyComplete();
        }
    }
}
