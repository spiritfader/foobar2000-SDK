#import "NSComboBox+fb2k.h"

namespace fb2k {
    void comboSetupHistory(NSComboBox * box, cfg_dropdown_history & var) {
        [box removeAllItems];
        pfc::string8 temp; var.get_state( temp );
        NSString * str = [NSString stringWithUTF8String: temp.c_str()];
        if ( str.length == 0 ) return;
        NSArray * arr = [str componentsSeparatedByCharactersInSet: NSCharacterSet.newlineCharacterSet];
        if ( arr.count == 0 ) return;
        for( NSString * str in arr ) {
            if ( str.length > 0 ) [box addItemWithObjectValue: str];
        }
        box.stringValue = arr.firstObject;
        
        [box.menu addItem: NSMenuItem.separatorItem];
    }

    void comboAddToHistory(NSComboBox * box, cfg_dropdown_history & var) {
        NSString * str = box.stringValue;
        if ( str.length > 0 ) var.add_item( str.UTF8String );
    }
} // namespace fb2k
