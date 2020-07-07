var k;
function handle() {
           $(this).css('background-color', "red");
            setInterval(req, 1000);
            function req(){
              $.getJSON("/data.json",
               function(data,status){
                   var updateLast = data.last;
                     var updateMax = data.max;
                     var updateAvg = data.avg;
                     $("#last").text(updateLast);
                     $("#max").text(updateMax);
                     $("#avg").text(updateAvg);
                 }
               );
        }
            
        }
           $("#thebutton").click(handle);