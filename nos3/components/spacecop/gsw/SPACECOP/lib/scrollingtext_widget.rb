require 'cosmos'
require 'cosmos/tools/tlm_viewer/widgets/widget'
require 'cosmos/tools/tlm_viewer/widgets/aging_widget'

module Cosmos
  # Accumulates the historical values of a telemetry item and renders them as a
  # scrolling text log with the newest value at the top. Subscribes directly to
  # packet data so no packets are missed when telemetry arrives faster than the GUI updates.
  class ScrollingtextWidget < Qt::TextEdit
    include Widget
    include AgingWidget

    def initialize(parent_layout, target_name, packet_name, item_name, width = 200, height = 100, value_type = :CONVERTED)
        super(target_name, packet_name, item_name, value_type)
        setup_aging
        setFixedSize(width.to_i, height.to_i)
        setReadOnly(true)
        parent_layout.addWidget(self) if parent_layout

        # Buffer of formatted historical values shown in the widget
        @historical_values = []

        # Subscribe to packet updates
        # The GUI updates at a different rate than telemetry, so relying
        # on the value= function alone means we miss some packets if telemetry
        # is arriving faster than the GUI updates
        @packet_subscriber_id = subscribe_packet_data([['SPACECOP', 'SPACECOP_REPORT_TLM']])
    end

    def format_value(data)
      data.to_s
    end

    def value=(data)
      scroll_pos = self.verticalScrollBar.value
      begin
        packet = get_packet(@packet_subscriber_id, non_block=true)
      rescue
        return
      end

      while packet do
          data = packet.read('IDS_MSG')
          @historical_values.push(super(data, format_value(data)))
          # Reverse historical values to show newest at the top
          self.text = @historical_values.reverse.join("\n")
          self.setColors(@foreground, @background)
          self.verticalScrollBar.value = scroll_pos
          begin
            packet = get_packet(@packet_subscriber_id, non_block=true)
          rescue
            # Queue is empty - no more historical packets
            return
          end
      end
    end

    def process_settings
      super
      process_aging_settings
    end
  end
end