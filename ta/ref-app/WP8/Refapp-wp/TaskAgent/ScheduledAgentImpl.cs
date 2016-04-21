/*  
    Copyright (c) 2012 Microsoft Corporation.  All rights reserved. 
    Use of this sample source code is subject to the terms of the Microsoft license  
    agreement under which you licensed this sample source code and is provided AS-IS. 
    If you did not accept the terms of the license agreement, you are not authorized  
    to use this sample source code.  For the terms of the license, please see the  
    license agreement between you and Microsoft. 
   
    To see all Code Samples for Windows Phone, visit http://go.microsoft.com/fwlink/?LinkID=219604  
   
*/
using System;
using System.Diagnostics;
using System.IO;
using System.Xml.Serialization;
using Microsoft.Phone.Networking.Voip;
using Microsoft.Phone.Scheduler;

namespace TaskAgent
{
    public class ScheduledAgentImpl : ScheduledTaskAgent
    {
        /// <remarks> 
        /// ScheduledAgent constructor 
        /// </remarks> 
        public ScheduledAgentImpl()
        {
        }

        /// <summary> 
        /// Agent that runs a scheduled task 
        /// </summary> 
        protected override void OnInvoke(ScheduledTask task)
        {
            this.Complete();
        }

        // This is a request to complete this agent 
        protected override void OnCancel()
        {
            this.Complete();
        }

        // This method is called when the incoming call processing is complete 
        private void OnIncomingCallDialogDismissed()
        {
            this.Complete();
        }

        // Complete this agent. 
        private void Complete()
        {
            base.NotifyComplete();
        }
    }
}