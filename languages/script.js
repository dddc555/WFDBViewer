var fs = require('fs')

function WordCount(temp, token) {
    var regex = new RegExp(token, "g")
    var count = (temp.match(regex) || []).length;
    var index = 0;
    // console.log(count);
    return count;
}
var getWordBetween = (srctext, between1, between2) => {
    try {
        var start = srctext.indexOf(between1) + between1.length;
        var end = srctext.indexOf(between2)

        var result = srctext.slice(start, end)
        return result;
    } catch (e) {
        console.log(e, srctext, between1, between2)
        return ''
    }
    // var regex = new RegExp(`${between1}(.*)${between2}`, "g")
    // console.log(regex)
    // var count = srctext.match(regex) || [];

    // return count;
}
// console.log(getWordBetween('<translation type="vanished">信号を整理</translation>', '<translation type="vanished">', '</translation>'))
// console.log(getWordBetween('<source>Load values from text file</source>', '<source>', '</source>'))
// console.log(getWordBetween('My cow always gives milk', 'cow', 'milk'))
var message = function (_source, _translation, _index) {
    this.source = _source
    this.translation = _translation;
    this.index = _index;
}
message.prototype.toString = function () {
    return `    <message>
        <source>${this.source}</source>
        <translation type="vanished">${this.translation}</translation>
    </message>`
}

var stringToMessageList = (contents) => {
    var list = contents.split('\n');
    var result = []
    list.forEach((line, index) => {
        if (line.includes('<source>')) {
            var source = getWordBetween(line, '<source>', '</source>')

            var translateString = list[index + 1];
            var trans = getWordBetween(translateString, '<translation type="vanished">', '</translation>')

            var msg = new message(source, trans, index);

            result.push(msg);

        }
    });
    return result;
}

var isDuplicated = function (list, search) {
    for (var i = 0; i < list.length; i++) {
        var item = list[i];
        if (item.source == search.source) return true;
    }

    return false;
}

var contents = fs.readFileSync('translation_clean_jp.ts', 'utf-8');
var list = contents.split('\n');
var msglist = stringToMessageList(contents);

var newMsgList = [];
msglist.forEach((msg, index) => {
    // console.log(msg.toString())

    if (!isDuplicated(newMsgList, msg)) {
        newMsgList.push(msg)
    } else {
        console.log(msg.source, msg.index)
    }
})


var resultFile = 'translation_cleaned_jp.ts'
var xml_header = `<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="en">
<context>
    <name></name>
`
var xml_bottom = `</context>
</TS>
`
fs.writeFileSync(resultFile, xml_header, 'utf-8');
newMsgList.forEach(msg => {
    fs.appendFileSync(resultFile, msg.toString(),'utf-8');
})
fs.appendFileSync(resultFile, xml_bottom, 'utf-8');