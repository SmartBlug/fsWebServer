const formToJSON = elements => [].reduce.call(elements, (data, element) => {
  if ((element.name)&&(element.name[0]!='_')&&(element.type!="checkbox")) data[element.name] = element.value;
  if (element.type=="checkbox") data[element.name] = element.checked;
  return data;
}, {});

const handleFormSubmit = event => {
  event.preventDefault();

  // Get all keys except if "_"
  result = formToJSON(event.target.elements);
  //console.log(event.target.elements);
  //console.log(result);
  // Handle arrays
  for (var i=0; i<event.target.elements.length; i++) {
    //console.log(event.target.elements[i].name,event.target.elements[event.target.elements[i].name].length,event.target.elements[i].parentNode.parentNode.getAttributeNode('name'));
    if (event.target.elements[i].getAttributeNode('name') && event.target.elements[i].parentNode.parentNode.getAttributeNode('name')) {
      let name = event.target.elements[i].name.substr(1);
      let group = event.target.elements[i].parentNode.parentNode.getAttributeNode('name').value;
      let multiple = event.target.elements[event.target.elements[i].name].length;
      //console.log(event.target.elements[event.target.elements[i].name].nodeName);
      if ((!multiple)||(event.target.elements[event.target.elements[i].name].nodeName=="SELECT")) {
        multiple = 1;
        //console.log(name,group,event.target.elements[event.target.elements[i].name].nodeName);
      }
      //else console.log(name,group,event.target.elements[event.target.elements[i].name][0].nodeName);
      if (!result[group]) {
        result[group]=[];
        for(let j=0;j<multiple;j++) result[group][j]={}
      }
      //console.log(event.target.elements[i],multiple);
      for(let j=0;j<multiple;j++) 
        //if (event.target.elements[event.target.elements[i].name][0].nodeName=="SELECT") result[group][j][name]=parseInt(event.target.elements[event.target.elements[i].name][j].value);
        //else 
        if (multiple>1) result[group][j][name]=event.target.elements[event.target.elements[i].name][j].value;
                   else result[group][j][name]=event.target.elements[event.target.elements[i].name].value;
      //if (!result[name][i]) result[name][i]={};
      //result[name][i]['value'] = "yes";
    }
  }
  //console.log(result);

  const request = fetch (event.target.action, {
    headers: {'Content-type': 'application/json'},
    method: 'POST',
    body: JSON.stringify(result)
  });
  event.target.querySelector('input[type="submit"]').disabled = true;
};

const forms = document.getElementsByTagName('form');
parentElement = forms[0].parentElement;
var newElement = document.createElement("input");
newElement.type = "label";
newElement.setAttribute('readonly', true);
newElement.className += "version";
newElement.name = "ver";
//parentElement.insertBefore(newElement, forms[0]);
forms[0].appendChild(newElement);

for (let i=0; i<forms.length; i++) {
  if (forms[i].getAttribute('load')) {
    fetch(forms[i].getAttribute('load')).then(res => res.json()).then(function(data) {
      for (let key in data){
        console.log(key,typeof data[key]);
        var items = document.getElementsByName(key);
        if (typeof data[key]=="object") {
          // array, we need to use template
          //template = document.getElementsByName(key+"_template")[0].cloneNode(true);
          //console.log(template,items);
          for (let j=0; j<data[key].length; j++) {
            //var newElement = document.createElement("li");
            template = document.getElementsByName(key+"_template")[0].cloneNode(true);
            template.removeAttribute("class");
            template.removeAttribute("name");
            //console.log(typeof template);

            for (var arraykey in data[key][j]) {
              //template.getElementsByName(arraykey).value = data[key][j][arraykey];
              template.childNodes.forEach(function(item) {
                if (item.name == '_'+arraykey) {
                  item.setAttribute("value",data[key][j][arraykey]);
                  //console.log(item.nodeName);
                  if (item.nodeName=="SELECT") {
                    for (let o=0;o<item.options.length;o++) {
                      //console.log(item.options[o].value);
                      if (item.options[o].value==data[key][j][arraykey]) item.selectedIndex = o;
                    }
                    //item.selectedIndex = 1;
                  }
                  //console.log(item,item.value,data[key][j][arraykey])
                }
              });
            }
            items[0].appendChild(template);
            /*console.log(template.childNodes.length);
            for (var k=0;k<template.childNodes.length;k++) {
              console.log(template.childNodes[k]);
              newElement.appendChild(template.childNodes[k]);
            }*/
            //template.forEach(function(item){
            //  newElement.appendChild(item);
            //});
            //console.log(data[key][j],template);
            
          }
        }
        else {
          // Normal items
          if (items) {
            //console.log(items,items[0].type);
            if (items.length) {
              if (items[0].type=="checkbox") items[0].checked = data[key];
              else {
                for (var i=0; i<items.length; i++) {
                  items[i].value = data[key];
                }
              }
            }
          }
        }
      }
      addFormListeners(document);
    });
  }
  if (forms[i].className=="formJSON") forms[i].addEventListener('submit', handleFormSubmit);
}

const handleFormChange = event => {
  for (var i=0; i<event.path.length; i++) {
    if (event.path[i].localName == "form") {
      item = event.path[i];
    }
  }
  item.querySelector('input[type="submit"]').disabled = false;
};

function addFormListeners(item) {
  const inputs = item.querySelectorAll('input,select')
  for (var i=0; i<inputs.length; i++) {
    if (item==document) {
      if (inputs[i].getAttribute('logo')) {
        parentElement = inputs[i].parentElement;
        var newElement = document.createElement("i");
        newElement.className += inputs[i].getAttribute('logo');
        parentElement.insertBefore(newElement, inputs[i]);
      }
    }
    inputs[i].addEventListener("keydown", handleFormChange);
    inputs[i].addEventListener("change", handleFormChange);
  }
  
  const groups = item.getElementsByClassName('group')
  for (var i=0; i<groups.length; i++) {
    if (groups[i].getAttribute('label')) {
      parentElement = groups[i].parentElement;
      var newElement = document.createElement("t");
      newElement.appendChild(document.createTextNode(groups[i].getAttribute('label')));
      parentElement.insertBefore(newElement, groups[i]);
    }
  }
}

function deleteItem(item) {
  form = item.parentNode.parentNode.parentNode;
  console.log(item.parentNode,item.parentNode.parentNode.parentNode);
  item.parentNode.parentNode.removeChild(item.parentNode);
  form.querySelector('input[type="submit"]').disabled = false;
}

function addItem(key) {
  items = document.getElementsByName(key);
  template = document.getElementsByName(key+"_template")[0].cloneNode(true);
  template.removeAttribute("class");
  template.removeAttribute("name");
  template.childNodes.forEach(function(item) {
    if (item.name) {
      item.setAttribute("value","");
      //console.log(item,item.value,data[key][j][arraykey])
    }
  });
  items[0].appendChild(template);
  addFormListeners(template);
}