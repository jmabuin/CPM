include ./Makefile.common

.PHONY: MonitoringAgent MonitoringMaster MonitoringClient scripts clean

all: MonitoringAgent MonitoringMaster MonitoringClient scripts
	@echo "================================================================================"
	@echo "CPM has been built."
	@echo "Location    = $(LOCATION)/$(BUILD_DIR)/"
	@echo "================================================================================"

MonitoringAgent:
	$(MAKE) -C $(AGENT_DIR)
	if [ ! -d "$(BUILD_DIR)" ]; then mkdir $(BUILD_DIR); fi
	cp $(AGENT_DIR)/MonitoringAgent $(BUILD_DIR)

MonitoringMaster:
	$(MAKE) -C $(MASTER_DIR)
	if [ ! -d "$(BUILD_DIR)" ]; then mkdir $(BUILD_DIR); fi
	cp $(MASTER_DIR)/MonitoringMaster $(BUILD_DIR)

MonitoringClient:
	cd $(CLIENT_DIR) && $(QMAKE)
	$(MAKE) -C $(CLIENT_DIR)
	if [ ! -d "$(BUILD_DIR)" ]; then mkdir $(BUILD_DIR); fi
	cp $(CLIENT_DIR)/MonitoringProcesses $(BUILD_DIR)

scripts:
	if [ ! -d "$(BUILD_DIR)" ]; then mkdir $(BUILD_DIR); fi
	cp $(SCRIPTS_DIR)/* $(BUILD_DIR)
	chmod +x $(BUILD_DIR)/*.sh

monitors: MonitoringAgent MonitoringMaster scripts
	@echo "================================================================================"
	@echo "Clients have been built."
	@echo "Location    = $(LOCATION)/$(BUILD_DIR)/"
	@echo "================================================================================"

clean:
	$(RMR) $(BUILD_DIR)
	$(MAKE) clean -C $(AGENT_DIR)
	$(MAKE) clean -C $(MASTER_DIR)
	$(MAKE) clean -C $(CLIENT_DIR)
	
