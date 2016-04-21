package com.webex.wme.test;

import junit.framework.TestCase;
import com.webex.wme.WmeError;

public class WmeErrorTest extends TestCase {

	public void testSuccess(){
		assertTrue(WmeError.Succeeded(0));
		assertTrue(WmeError.Failed(-1));
		assertTrue(WmeError.Succeeded(100));
		assertTrue(WmeError.Failed(WmeError.E_FAIL));
		assertTrue(WmeError.Succeeded(WmeError.S_OK));
		assertTrue(WmeError.Succeeded(WmeError.S_FALSE));
		assertTrue(WmeError.Failed(WmeError.E_OUTOFMEMORY));
		assertTrue(WmeError.Failed(WmeError.E_INVALIDARG));
		assertTrue(WmeError.Failed(WmeError.E_NOTIMPL));
		assertTrue(WmeError.Failed(WmeError.E_NOINTERFACE));
		assertTrue(WmeError.Failed(WmeError.E_POINTER));
		assertTrue(WmeError.Failed(WmeError.E_VIDEO_CAMERA_FAIL));
		assertTrue(WmeError.Failed(WmeError.E_NEGOTIATION));
		assertTrue(WmeError.Failed(WmeError.E_INVALIDSTATUS));
	}
}
