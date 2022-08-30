import * as Vue from 'https://cdn.jsdelivr.net/npm/vue@3.2.37/dist/vue.esm-browser.prod.js';
document.addEventListener('DOMContentLoaded', async (evt) => {
  loadStarryDashboardApp_(evt);
});

const starry_option = {
  data() {
    return {
      mansionMatrix:[]
    };
  },
  methods: {
    onClickGotoBtn (evt) {
      console.log('onClickGotoBtn::evt=<',evt,'>');
      onGotoMansionFunc(evt.target,this);
    }
  }  
}

const loadStarryDashboardApp_ = (evt) => {
  const appStarry = Vue.createApp(starry_option);
  const starryVM = appStarry.mount('#vue-ui-starry-dashboard');
  const starMansionListStr = localStorage.getItem(constMansionList);
  if(starMansionListStr) {
    const starMansionList = JSON.parse(starMansionListStr);    
    console.log('loadStarryDashboardApp_::starMansionList=<',starMansionList,'>');
    if(starMansionList.length > 0) {
      const matrix = array2Matrix(starMansionList,2);
      starryVM.mansionMatrix = matrix;
    }
  }
}

const onGotoMansionFunc = (elem,mansion) => {
  console.log('onGotoMansionFunc::elem=<',elem,'>');
  const address = elem.getAttribute('mansion');
  console.log('onGotoMansionFunc::address=<',address,'>');
  localStorage.setItem(constMansionTargetAddress,address);
  const href = elem.getAttribute('href');
  console.log('onGotoMansionFunc::href=<',href,'>');
  location = href;
}



const array2Matrix = (array,rowSize)=> {
  const matrix = [];
  let row = [];
  for(const elem of array) {
    row.push(elem);
    if(row.length >= rowSize) {
      matrix.push(row);
      row = [];
    }
  }
  if(row.length > 0) {
    matrix.push(row);
  }
  return matrix;
}


