// Copyright 2020 Oath Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.config.provision.zone;

/**
 * The routing methods supported by a zone.
 *
 * @author mpolden
 */
public enum RoutingMethod {

    /** Routing happens through shared routing layer */
    shared,

    /** Routing happens through a dedicated layer 4 load balancer */
    exclusive,

}
