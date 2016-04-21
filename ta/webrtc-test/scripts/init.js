(function(){
	"use strict";
	window.yams = {};
	
    function fDate(iD){
        if(iD < 10 && iD >= 0){
            return '0' + iD;
        }else{
            return iD;
        }
    };
	
	function getTime(method){
        var now = new Date();
        var prefix = "[" + fDate(now.getMonth() + 1) + "/" + fDate(now.getDate());
        prefix += " " + fDate(now.getHours()) + ":" + fDate(now.getMinutes()) + ":" + fDate(now.getSeconds()) + "." + fDate(now.getMilliseconds()) + "]";
        prefix += "[" + method + "]";    
        return prefix;
    };
	
    var getsettest = {};
    Object.defineProperty(getsettest, 'test', {
        get: function() {
            return 'test';
        }});
    var hasGetSet = getsettest.test == 'test';
	var logger = {};
    var _console = window.console || {};
	var _methods = ['log', 'info', 'warn', 'error', 'debug'];
    for (var i = 0, len = _methods.length; i < len; i++) {
		(function(method){
		if(!_console[method])
			_console[method] = function() {};
        var fn = _console[method];
        var supportBind = false;
        try{
            Function.prototype.bind.call(_console[method], _console);
            supportBind = true;
        }catch(e){
        }
		
        if (supportBind && Function.prototype.bind && hasGetSet) {
            if(window.useSawBucks){
                Object.defineProperty(logger, method, {
                    get: function() {
                        return Function.prototype.bind.call(_console[method], _console);
                    }
                });
            }
            else{
                Object.defineProperty(logger, method, {
                    get: function() {
                        return Function.prototype.bind.call(_console[method], _console, getTime(method));
                    }
                });
            }
        }else{
            //Only IE7/8 goes here, so it won't be the case for whitney.
            logger[method] = function() { 
                var args = Array.prototype.slice.call(arguments, 0);
                args.unshift(getTime(method));
                Function.prototype.apply.call(_console[method], _console, args);
            };
        }
		}(_methods[i]));
    }
	window.yams.logger = logger;

	logger.info("Yams logger is inited.");
	
	window.yams.uuid = function(){
        return 'xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx'.replace(/[xy]/g, function(c) {
					var r = Math.random()*16|0, v = c == 'x' ? r : (r&0x3|0x8);
                    return v.toString(16);
                });
    };
	
	window.yams.urlParams = (function(a) {
		if (a == "") return {};
		var b = {};
		for(var i = 0; i < a.length; ++i)
		{
			var p = a[i].split('=');
			if(p.length != 2) 
				continue;
			b[p[0]] = decodeURIComponent(p[1].replace(/\+/g, " "));
		}
		return b;
	}(window.location.search.substr(1).split('&')));
}());
