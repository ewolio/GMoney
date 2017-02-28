import QtQuick 2.0
import QtGraphicalEffects 1.0
import "colorMisc.js" as ColorMisc

Item {
    id: root
    width: 200
    height: 125

    property color color: 'red';
    signal editingFinished

    onColorChanged:{
        var hsv = ColorMisc.rgb2hsv(color.r,color.g,color.b);
        if(hsv.s==0){
            hsv.h = hueWheel.angle;
            if(hsv.v==0)
                hsv.s = saturationSelector.value;
        }

        saturationSelector.topColor = Qt.hsva(hsv.h, 1, hsv.v, 1);
        saturationSelector.bottomColor = Qt.hsva(hsv.h, 0, hsv.v, 1);
        valueSelector.topColor   = Qt.hsva(hsv.h, hsv.s, 1, 1);
        valueSelector.bottomColor   = Qt.hsva(hsv.h, hsv.s, 0, 1);
        hueWheel.angle = hsv.h;
        valueSelector.value = hsv.v;
        saturationSelector.value = hsv.s;
    }

    function updateColor(){
        var h = hueWheel.angle,
            s = saturationSelector.value,
            v = valueSelector.value;
        color = Qt.hsva(h,s,v,1);
        editingFinished();
    }

    GGradiantSelector{
        id: valueSelector
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        labelVisible: false
        gaugeWidth: 15

        bottomColor: "black"
        topColor: "red"
        value: 1


        onEditingFinished: root.updateColor()
    }

    GWheelHueSelector{
        id: hueWheel
        anchors.bottom: parent.bottom
        anchors.top: parent.top
        anchors.left: valueSelector.right
        anchors.right: saturationSelector.left

        onEditingFinished: root.updateColor()

        Rectangle{
            width: (hueWheel.wheelRadius - hueWheel.wheelWidth) * 2 * 0.7
            height: width
            anchors.centerIn: parent
            radius: width
            color: root.color
        }
    }

    GGradiantSelector{
        id: saturationSelector
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        labelVisible: false
        gaugeWidth: 15

        topColor: 'red'
        value: 1

        onEditingFinished: root.updateColor()
    }


}
