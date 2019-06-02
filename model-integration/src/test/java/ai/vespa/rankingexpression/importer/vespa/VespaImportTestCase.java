// Copyright 2019 Oath Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package ai.vespa.rankingexpression.importer.vespa;

import ai.vespa.rankingexpression.importer.ImportedModel;
import ai.vespa.rankingexpression.importer.configmodelview.ImportedMlFunction;
import org.junit.Test;

import java.util.List;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

/**
 * @author bratseth
 */
public class VespaImportTestCase {

    @Test
    public void testExample() {
        ImportedModel model = importModel("example");

        assertEquals(2, model.inputs().size());
        assertEquals("tensor(name{},x[3])", model.inputs().get("input1").toString());
        assertEquals("tensor(x[3])", model.inputs().get("input2").toString());

        assertEquals(2, model.smallConstants().size());
        assertEquals("tensor(x[3]):{{x:0}:0.5,{x:1}:1.5,{x:2}:2.5}", model.smallConstants().get("constant1"));
        assertEquals("tensor():{3.0}", model.smallConstants().get("constant2"));

        assertEquals(1, model.largeConstants().size());
        assertEquals("tensor(x[3]):{{x:0}:0.5,{x:1}:1.5,{x:2}:2.5}", model.largeConstants().get("constant1asLarge"));

        assertEquals(2, model.expressions().size());
        assertEquals("max(reduce(input1 * input2, sum, name) * constant1,x) * constant2",
                     model.expressions().get("foo1").getRoot().toString());
        assertEquals("max(reduce(input1 * input2, sum, name) * constant1asLarge,x) * constant2",
                     model.expressions().get("foo2").getRoot().toString());

        List<ImportedMlFunction> functions = model.outputExpressions();
        assertEquals(2, functions.size());
        ImportedMlFunction foo1Function = functions.get(0);
        assertEquals(2, foo1Function.arguments().size());
        assertTrue(foo1Function.arguments().contains("input1"));
        assertTrue(foo1Function.arguments().contains("input2"));
        assertEquals(2, foo1Function.argumentTypes().size());
        assertEquals("tensor(name{},x[3])", foo1Function.argumentTypes().get("input1"));
        assertEquals("tensor(x[3])", foo1Function.argumentTypes().get("input2"));
    }

    @Test
    public void testEmpty() {
        ImportedModel model = importModel("empty");
        assertTrue(model.expressions().isEmpty());
        assertTrue(model.functions().isEmpty());
        assertTrue(model.inputs().isEmpty());
        assertTrue(model.largeConstants().isEmpty());
        assertTrue(model.smallConstants().isEmpty());
    }

    @Test
    public void testWrongName() {
        try {
            importModel("misnamed");
            fail("Expected exception");
        }
        catch (IllegalArgumentException e) {
            assertEquals("Model 'expectedname' must be saved in a file named 'expectedname.model'", e.getMessage());
        }
    }

    private ImportedModel importModel(String name) {
        String modelPath = "src/test/models/vespa/" + name + ".model";

        VespaImporter importer = new VespaImporter();
        assertTrue(importer.canImport(modelPath));
        ImportedModel model = new VespaImporter().importModel(name, modelPath);
        assertEquals(name, model.name());
        assertEquals(modelPath, model.source());
        return model;
    }

}
