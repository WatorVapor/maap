export class Mass {
  static debug = false;
  constructor(storePrefix) {
    if(storePrefix) {
      this.secretKeyPath_ = `${storePrefix}/secretKey`;
      this.publicKeyPath_ = `${storePrefix}/publicKey`;
      this.addressPath_ = `${storePrefix}/address`;
      const result = this.loadMassKey_();
      if(!result) {
        this.createMassKey_();
        this.loadMassKey_();
      }
    }
  }
  load(secretKey) {
    if(Mass.debug) {
      console.log('Mass::load::secretKey=<',secretKey,'>');
    }
    const secretBin = nacl.util.decodeBase64(secretKey);
    if(Mass.debug) {
      console.log('Mass::load::secretBin=<',secretBin,'>');
    }
    const keyPair = nacl.sign.keyPair.fromSecretKey(secretBin);
    if(Mass.debug) {
      console.log('Mass::load::keyPair=<',keyPair,'>');
    }
    this.secretKey_ = keyPair.secretKey;
    this.publicKey_ = keyPair.publicKey;
    const b64Pub = nacl.util.encodeBase64(keyPair.publicKey);
    if(Mass.debug) {
      console.log('Mass::load:b64Pub=<',b64Pub,'>');
    }
    this.publicKeyB64_ = b64Pub;
    const address = this.calcAddress_(b64Pub);
    this.address_ = address;
    return address;
  }
  pub() {
    return this.pubKeyB64_;
  }
  secret() {
    return this.priKeyB64_;
  }
  address() {
    return this.address_;
  }
  destory() {
    localStorage.removeItem(this.secretKeyPath_);
    localStorage.removeItem(this.publicKeyPath_);
    localStorage.removeItem(this.addressPath_);
  }
  verifySecretKey(secretKey) {
    if(Mass.debug) {
      console.log('Mass::verifySecretKey::secretKey=<',secretKey,'>');
    }
    const secretBin = nacl.util.decodeBase64(secretKey);
    if(Mass.debug) {
      console.log('Mass::verifySecretKey::secretBin=<',secretBin,'>');
    }
    const keyPair = nacl.sign.keyPair.fromSecretKey(secretBin);
    if(Mass.debug) {
      console.log('Mass::verifySecretKey::keyPair=<',keyPair,'>');
    }
    if(keyPair) {
      return true;
    }
    return false;
  }
  importSecretKey(secretKey) {
    if(Mass.debug) {
      console.log('Mass::importSecretKey::secretKey=<',secretKey,'>');
    }
    const secretBin = nacl.util.decodeBase64(secretKey);
    if(Mass.debug) {
      console.log('Mass::importSecretKey::secretBin=<',secretBin,'>');
    }
    const keyPair = nacl.sign.keyPair.fromSecretKey(secretBin);
    if(Mass.debug) {
      console.log('Mass::importSecretKey::keyPair=<',keyPair,'>');
    }
    if(keyPair) {
      save2Storage_(keyPair);
      loadMassKey_();
      return true;
    }
    return false;
  }
  createMassKey_() {
    while(true) {
      const address = this.mineMassKey_();
      if(address.startsWith('mp')) {
        break;
      }
    }
  }
  mineMassKey_() {
    const keyPair = nacl.sign.keyPair();
    if(Mass.debug) {
      console.log('Mass::mineMassKey_:keyPair=<',keyPair,'>');
    }
    return this.save2Storage_(keyPair);
  }
  save2Storage_(keyPair){
    const b64Pri = nacl.util.encodeBase64(keyPair.secretKey);
    if(Mass.debug) {
      console.log('Mass::save2Storage_:b64Pri=<',b64Pri,'>');
    }
    localStorage.setItem(this.secretKeyPath_,b64Pri);    
    const b64Pub = nacl.util.encodeBase64(keyPair.publicKey);
    if(Mass.debug) {
      console.log('Mass::save2Storage_:b64Pub=<',b64Pub,'>');
    }
    localStorage.setItem(this.publicKeyPath_,b64Pub);
    const hash1Pub = CryptoJS.SHA1(b64Pub).toString(CryptoJS.enc.Base64);
    if(Mass.debug) {
      console.log('Mass::save2Storage_:hash1Pub=<',hash1Pub,'>');
    }
    const hash1pubBuffer = nacl.util.decodeBase64(hash1Pub);
    if(Mass.debug) {
      console.log('Mass::save2Storage_:hash1pubBuffer=<',hash1pubBuffer,'>');
    }
    const encoder = new base32.Encoder({ type: "crockford", lc: true });
    const address = encoder.write(hash1pubBuffer).finalize();
    if(Mass.debug) {
      console.log('Mass::save2Storage_:address=<',address,'>');
    }
    localStorage.setItem(this.addressPath_,address);
    return address;
  }
  loadMassKey_() {
    try {
      const address = localStorage.getItem(this.addressPath_);
      if(Mass.debug) {
        console.log('Mass::loadMassKey_:address=<',address,'>');
      }
      this.address_ = address;
      const PriKey = localStorage.getItem(this.secretKeyPath_);
      if(Mass.debug) {
        console.log('Mass::loadMassKey_:PriKey=<',PriKey,'>');
      }
      this.priKeyB64_ = PriKey;
      this.priKey_ = nacl.util.decodeBase64(PriKey);
      if(Mass.debug) {
        console.log('Mass::loadMassKey_:this.priKey_=<',this.priKey_,'>');
      }
      const keyPair = nacl.sign.keyPair.fromSecretKey(this.priKey_);
      if(Mass.debug) {
        console.log('Mass::loadMassKey_:keyPair=<',keyPair,'>');
      }    
      const pubKey = localStorage.getItem(this.publicKeyPath_);
      if(Mass.debug) {
        console.log('Mass::loadMassKey_:pubKey=<',pubKey,'>');
      }
      this.pubKeyB64_ = pubKey;
      this.pubKey_ = nacl.util.decodeBase64(pubKey);
    } catch(err) {
      console.log('Mass::loadMassKey_:err=<',err,'>');
      return false;
    }
    return true;
  }
  calcAddress_(b64Pub) {
    const hash1Pub = CryptoJS.SHA1(b64Pub).toString(CryptoJS.enc.Base64);
    if(Mass.debug) {
      console.log('Mass::load:hash1Pub=<',hash1Pub,'>');
    }
    const hash1pubBuffer = nacl.util.decodeBase64(hash1Pub);
    if(Mass.debug) {
      console.log('Mass::load:hash1pubBuffer=<',hash1pubBuffer,'>');
    }
    const encoder = new base32.Encoder({ type: "crockford", lc: true });
    const address = encoder.write(hash1pubBuffer).finalize();
    if(Mass.debug) {
      console.log('Mass::load:address=<',address,'>');
    }
    return address;
  }
}








